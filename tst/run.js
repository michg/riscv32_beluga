#!/usr/bin/node

"use strict"

var assert = require('assert')
var EventEmitter = require('events').EventEmitter,
    ee = new EventEmitter()
var fs = require('fs')
var path = require('path')
var spawn = require('child_process').spawn


var prgname = 'run.js'
var id, dir
var excludes = {}
var fails = []
var copts = [ '--error-stop=0', '--no-warning-code', '--unwind-typedef',
              '--warning-on=5',   '--warning-on=20',  '--warning-on=25',  '--warning-on=53',
              '--warning-on=55',  '--warning-on=78',
              '--warning-on=143', '--warning-on=173', '--warning-on=213', '--warning-on=226',
              '--warning-on=253' ]
var run = {
    'beluga\'s diagnostics': {
        proc:  diagout,
        exec:  '../../build/beluga',
        copts: copts,
        eopts: [ '-Wv', '--std=c90', '--input-charset=utf8', '--exec-charset=utf8' ],
        touts: [ null, false, true ]      // stderr only
    },
    'preprocessor': {
        proc:  diagout,
        exec:  '../../build/beluga',
        copts: [ '-E' ].concat(copts),
        eopts: [ '-Wv', '--std=c90', '--input-charset=utf8', '--exec-charset=utf8' ],
        touts: [ null, true, true ]
    },
    'mcpp\'s testcases': {
        proc:  diagout,
        exec:  '../../build/beluga',
        copts: [ '-E' ].concat(copts),
        eopts: [ '-Wv', '--std=c90' ],
        touts: [ null, true, true ]
    },
    'assembly output': {
        proc: evalasm,
        exec: '../../build/beluga'
    }
}


function prep() {
    var idx = (path.basename(process.argv[0]).indexOf('node') >= 0)? 1: 0

    var err = function (msg) {
        console.log(prgname+': '+ msg)
        process.exit(1)
    }

    if (!process.argv[idx]) prgname = path.basename(process.argv[idx])

    !process.argv[++idx] && err('no working directory given')
    dir = process.argv[idx]

    try {
        id = fs.readFileSync(path.join(dir, 'ID'), 'utf8')
    } catch(e) {
        err('no test identification set')
    }
    id = id.replace(/\n/g, '')
}


function exclude() {
    var supportLL = function () {
        var lines, ll

        try {
            lines = fs.readFileSync(path.join(__dirname, '..', 'build', 'conf.lst'), 'utf8')
        } catch(e) {
            return false
        }

        lines.split('\n')
             .filter(function (l) { return (l[0] === '=') })
             .forEach(function (l) {
                 if (l.indexOf('SUPPORT_LL') === 1) ll = true
             })

        return ll
    }

    var handle = function (fn, by) {
        var buf

        try {
            buf = fs.readFileSync(path.join(dir, fn), 'utf8')
        } catch(e) {
            return
        }

        buf = buf.split('\n')
        console.log('\n- following files will not be used'+(by || '')+':')
        for (var i = 0; i < buf.length; i++) {
            if (buf[i]) {
                excludes[buf[i]] = true
                console.log('  '+buf[i])
            }
        }
    }

    handle('EXCLUDE')
    if (supportLL()) handle('EXCLUDE.LL', ' by SUPPORT_LL')
    else handle('EXCLUDE.L', ' by !SUPPORT_LL')
}


function next(name, fail, msg) {
    if (fail) {
        process.stdout.write('[FAILED]'+((msg)? ': '+msg: '')+'\n')
        fails.push(name)
    } else {
        process.stdout.write('[ok]\n')
    }
    ee.emit('next')
}


function getlines(buf) {
    var s = 0;
    var lines = []

    for (var i = 0; i < buf.length; i++) {
        if (buf[i] === 10) {
            lines.push(buf.slice(s, i+1))
            s = i + 1
        }
    }

    return lines
}


function bufeq(b1, b2) {
    var cmp = function (a, b) {
        return a.compare(b)
    }

    if (b1.length !== b2.length) return false

    b1 = getlines(b1).sort(cmp)
    b2 = getlines(b2).sort(cmp)
    if (b1.length !== b2.length) return false

    for (var i = 0; i < b1.length; i++) {
        if (!b1[i].equals(b2[i])) return false
    }

    return true
}


function diagout(name) {
    var opts
    var codes

    process.stdout.write('  checking for '+name+'... ')

    try {
        codes = fs.readFileSync(path.join(dir, name), 'binary')
    } catch(e) {
        next(name, true, 'cannot read test code')
        return
    }

    codes = codes.split('\n')
    if (codes[0].substring(0, 2) === '/*' &&
        (opts=/\/\*(.*) \*\//.exec(codes[0]), opts && opts[1])) {
        opts = opts[1].split(' -')
        for (var i = 0; i < opts.length; ) {
            if (!opts[i]) {
                opts.splice(i, 1)
                continue
            } else {
                opts[i] = ('-'+opts[i])
            }
            i++
        }
    } else {
        opts = run[id].eopts
    }
    opts = opts.concat(run[id].copts)
    opts.push('./'+name)
    codes = null

    !function () {
        var child
        var stdout = new Buffer(0),
            stderr = new Buffer(0)

        child = spawn(run[id].exec, opts, { cwd: dir })
        child.stdout.on('data', function (data) {
            stdout = Buffer.concat([ stdout, data ])
        })
        child.stderr.on('data', function (data) {
            stderr = Buffer.concat([ stderr, data ])
        })

        child.on('close', function () {
            var fail, msg
            var sources = [ null, stdout, stderr ]
            var origins = []

            for (var i = 1; i <= 2; i++) {
                if (!run[id].touts[i]) continue
                try {
                    origins[i] = fs.readFileSync(path.join(dir, name+'.'+i+'.out'))
                } catch(e) {
                    origins[i] = undefined
                }
                if (!origins[i] || !bufeq(sources[i], origins[i])) {
                    fail = true
                    try {
                        fs.writeFileSync(path.join(dir, name+'.'+i+'.out.new'), sources[i])
                    } catch(e) {
                        msg = 'cannot write output file'
                    }
                }
            }

            next(name, fail, msg)
        })
    }()
}


function evalasm(name) {
    var count = 0
    var targets = [ 'x86-test', 'x86-linux' ]
    var fail = false, msg = []

    process.stdout.write('  checking for '+name+'... ')

    for (var i = 0; i < targets.length; i++) {
        !function (target) {
            var child
            var opts = []
            var stdout = new Buffer(0)

            opts.push('--target='+target, './'+name)
            child = spawn(run[id].exec, opts, { cwd: dir })
            child.stdout.on('data', function (data) {
                stdout = Buffer.concat([ stdout, data ])
            })

            child.on('close', function () {
                var origin

                try {
                    origin = fs.readFileSync(path.join(dir, name+'.'+target))
                } catch(e) {
                    origin = undefined
                }
                if (!origin || !stdout.equals(origin)) {
                    fail = fail || true
                    try {
                        fs.writeFileSync(path.join(dir, name+'.'+target+'.s'), stdout)
                        msg.push(target)
                    } catch(e) {
                        msg.push('cannot write output file')
                    }
                }

                ;(++count === targets.length) && next(name, fail, msg.join(', '))
            })
        }(targets[i])
    }
}


// starts here
!function () {
    var buf
    var idx = 0
    var list = []

    prep()
    console.log('Running tests for '+id+':')

    exclude()
    buf = fs.readdirSync(dir)
    for (var i = 0; i < buf.length; i++) {
        ;(/[a-z0-9\-].c$/i.test(buf[i]) && !excludes[buf[i]]) && list.push(buf[i])
    }

    console.log('')
    ee.on('next', function () {
        if (idx < list.length) {
            run[id].proc(list[idx++])
        } else if (fails.length > 0) {
            console.log('\n- test failed for the following files:')
            for (var i = 0; i < fails.length; i++) {
                console.log('  '+fails[i])
            }
            process.exit(1)
        } else {
            console.log('\n- all '+list.length+' tests passed')
            process.exit(0)
        }
    }).emit('next')
}()

// end of run.js
