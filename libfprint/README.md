## libfprint demo

Compile [libfprint](https://github.com/freedesktop/libfprint/tree/master/examples) and [facil.io](https://github.com/boazsegev/facil.io).

Having both libraries compiled and available on your system, you can compile any of these .c files as follows:

```
$ gcc -Wall -o "enroll" "enroll.c" -I "/tmp/libfprint-V_1_0/libfprint/" -lm "/tmp/libfprint-V_1_0/builddir/libfprint/libfprint.so.0" -I "/tmp/libfprint-V_1_0/builddir/libfprint/" -Im "/tmp/facil.io-0.7.3/tmp/libfacil.so" -I "/tmp/facil.io-0.7.3/libdump/include/" -Wl,-rpath=/tmp/libfprint-V_1_0/builddir/libfprint/,-rpath=/tmp/facil.io-0.7.3/tmp/ 
```



**Change the paths to libfprint and facil.io accordingly with your local files**.

You can find further details about the compilation [here](https://fbentoluiz.io/libfprint).
