/* --input-charset=iso2022jp --exec-charset=iso2022kr --wide-exec-charset=ucs4 -Wv */
char x[] =
    "abcd$B@83h4A;z@83h4A;z(Bzzz"; char y[] = "abcd$B@83h4A;z@83h4A;z(B\xzzz"

    L"\xzzzabcd$B@83h4A;z@83h4A;z(B";
