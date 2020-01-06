About
-----

sunstat is a simple public domain application that shows sunset and 
sunrise based on your latitude and longitude.

The sunriset library is based on the excellent code by [Paul 
Schlyter](http://stjarnhimlen.se/). sunstat is based on the sun 
application by [Joachim Nilsson](https://github.com/troglobit).


Build
-----

Compilation follows the usual "./configure; make; make install" 
semantics.

```sh
$ ./configure
checking for C compiler... cc
checking for pledge... yes
creating Makefile... done
$ make
cc -DHAVE_CONFIG -Wall -Wextra -g -O2  -c sunstat.c
cc -o sunstat sunstat.o -lm
$ doas make install
install -d /usr/local/bin
install -s -o root -g bin -m 0755 sunstat /usr/local/bin
```


Usage
-----

```
$ sunstat
Usage:
  sunstat +/-latitude +/-longitude

Examples:
    sunstat +40.6611 -73.9439 (use $TZ || /etc/localtime)
    TZ='America/New_York' sunstat +40.6611 -73.9439
    TZ='UTC' sunstat +40.6611 -73.9439

$ # print today's sunrise/sunset times
$ # .. for Minneapolis
$
$ TZ='America/Chicago' sunstat +44.9819 -93.2692
              Sunrise     Sunset
              07:51 CST   16:45 CST
       Civil  07:17 CST   17:19 CST
    Nautical  06:40 CST   17:55 CST
Astronomical  06:05 CST   18:31 CST

              Daylight    Twilight
              08h54m42s   ---
       Civil  10h01m18s   00h33m17s
    Nautical  11h15m07s   01h10m12s
Astronomical  12h26m11s   01h45m44s

The Sun is overhead (due south/north) at 12:18 CST.
```

Compatibility
-------------

Tested on

  - OpenBSD 6.6

  - CentOS 8.1

  - Fedora 30

  - Debian 10.2

  - Alpine 3.11.2

  - macOS Catalina 10.15.1

  - NetBSD 8.1
