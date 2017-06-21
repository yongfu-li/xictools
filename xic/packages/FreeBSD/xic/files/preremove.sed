#! /bin/sh

if [ x"$PKG_PREFIX" = "x" ]; then
    prefix=/usr/local
else
    prefix=$PKG_PREFIX
fi

libdir=$prefix/xictools/xic
relnote=xic@VERSION@

# Back up config files
cfgfiles="device.lib \
    model.lib \
    mozyrc \
    xic_format_lib \
    xic_mesg \
    xic_stipples \
    xic_tech.hyp \
    xic_tech.scmos"

for a in $cfgfiles; do
    if [ -f $libdir/startup/$a ]; then
        cp -pf $libdir/startup/$a $libdir/startup/$a.tmporig
    fi
done

if [ -f $libdir/docs/$relnote ]; then
    cp -pf $libdir/docs/$relnote $libdir/docs/$relnote.tmp
fi
