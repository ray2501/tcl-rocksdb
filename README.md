tcl-rocksdb
=====

[RocksDB](http://rocksdb.org/) is a high performance embedded database
for key-value data. It is a fork of LevelDB which was then optimized to
exploit many central processing unit (CPU) cores, and make efficient use
of fast storage, such as solid-state drives (SSD), for input/output (I/O)
bound workloads. It is based on a log-structured merge-tree (LSM tree)
data structure.

This extension is the Tcl interface to the RocksDB.
Now tcl-rocksdb at an early development stage (or a prototype).


License
=====

RocksDB is Licensed under Apache License Version 2.0 and GPL v2 (dual license)
after 2017/07/16.

tcl-rocksdb is Licensed under Apache License, Version 2.0.


UNIX BUILD
=====

I only test on openSUSE.

The first step is to build RocksDB shared library and install.
RocksDB depends on newer gcc/clang with C++11 support.
For more details consider
[INSTALL.md](https://github.com/facebook/rocksdb/blob/master/INSTALL.md).

    $ make install-shared INSTALL_PATH=/usr

To uninstall use:

    $ make uninstall INSTALL_PATH=/usr

After install RocksDB shared library, you can build this extension.

Building under most UNIX systems is easy, just run the configure script
and then run make. For more information about the build process, see
the tcl/unix/README file in the Tcl src dist. The following minimal
example will install the extension in the /opt/tcl directory.

    $ export CC=g++
    $ cd tcl-rocksdb
    $ ./configure --prefix=/opt/tcl
    $ make
    $ make install
	
If you need setup directory containing tcl configuration (tclConfig.sh),
below is an example:

    $ export CC=g++
    $ cd tcl-rocksdb
    $ ./configure --with-tcl=/opt/activetcl/lib
    $ make
    $ make install


Implement commands
=====

The key and data is interpreted by Tcl as a byte array.

### Basic usage
rocksdb version

The command `rocksdb version` return a list of the form {major minor patch} 
for the major, minor and patch levels of the RocksDB release.

### Database
rocksdb open -path path ?-readonly BOOLEAN? ?-create_if_missing BOOLEAN? 
 ?-error_if_exists BOOLEAN? ?-paranoid_checks BOOLEAN? 
 ?-use_fsync BOOLEAN? ?-write_buffer_size size? 
 ?-max_write_buffer_number number? ?-target_file_size_base size? 
 ?-max_open_files number? ?-compression type?   
DB_HANDLE get key  
DB_HANDLE put key value ?-sync BOOLEAN?  
DB_HANDLE delete key  
DB_HANDLE exists key  
DB_HANDLE write BAT_HANDLE  
DB_HANDLE batch  
DB_HANDLE iterator  
DB_HANDLE close  
IT_HANDLE seektofirst  
IT_HANDLE seektolast  
IT_HANDLE seek key  
IT_HANDLE valid  
IT_HANDLE next  
IT_HANDLE prev  
IT_HANDLE key  
IT_HANDLE value  
IT_HANDLE close  
BAT_HANDLE put key value  
BAT_HANDLE delete key  
BAT_HANDLE close  

The command `rocksdb open` create a database handle. -path option is the path 
of the database to open.
-compression type supports "no", "snappy", "zlib", "bzip2", "lz4" and "lz4hc".

`DB_HANDLE iterator` create an Iterator handle.

`DB_HANDLE batch` create a WriteBatch handle. Users can use `DB_HANDLE write`
to apply a set of updates.


Examples
=====

A basic example:

    package require rocksdb

    set dbi [rocksdb open -path "./testdb" -create_if_missing 1]
    $dbi put "test1" "1234567890"
    set value [$dbi get "test1"]
    puts $value

    $dbi delete "test1"
    $dbi put "test2" "2345678901"
    $dbi put "test3" "3456789010"
    $dbi put "test4" "4567890102"
    $dbi put "test5" "5678901023"

    set it [$dbi iterator]
    for {$it seektofirst} {[$it valid] == 1} {$it next} {
        set key [$it key]
        set value [$it value]
        puts "Iterator --- I get $key: $value"
    }
    $it close
    $dbi close

WriteBatch example:

    package require rocksdb

    set dbi [rocksdb open -path "./testdb" -create_if_missing 1]
    set bat [$dbi batch]
    $bat put "test1" "1234567890"
    $bat put "test2" "2345678901"
    $bat put "test3" "3456789010"
    $bat put "test4" "4567890102"
    $bat put "test5" "5678901023"
    $dbi write $bat
    $bat close

    set it [$dbi iterator]
    for {$it seektofirst} {[$it valid] == 1} {$it next} {
        set key [$it key]
        set value [$it value]
        puts "Iterator --- I get $key: $value"
    }
    $it close
    $dbi close


