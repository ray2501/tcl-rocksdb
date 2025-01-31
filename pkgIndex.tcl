# -*- tcl -*-
# Tcl package index file, version 1.1
#
if {[package vsatisfies [package provide Tcl] 9.0-]} {
    package ifneeded rocksdb 0.4.0 \
	    [list load [file join $dir libtcl9rocksdb0.4.0.so] [string totitle rocksdb]]
} else {
    package ifneeded rocksdb 0.4.0 \
	    [list load [file join $dir librocksdb0.4.0.so] [string totitle rocksdb]]
}
