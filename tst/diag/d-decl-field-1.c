struct x { int (*m1)();
           char m2:2;
           unsigned short m3:8;
           int m4:4;
           unsigned int m5:4;
           signed int m6:4;
           unsigned int m7:x;
           unsigned int m8:-1;
           unsigned int m9:32;
           unsigned int m10:33;
           int m11:0;
           struct m12 { int x; };
           struct { int m13; int m16; };
           struct x m14;
           int m15(); };
