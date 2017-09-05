%{!?directory:%define directory /usr}

%define buildroot %{_tmppath}/%{name}-%{version}

Name:          tcl-rocksdb
Summary:       Tcl interface for RocksDB
Version:       0.3
Release:       1
License:       Apache License, Version 2.0
Group:         Development/Libraries/Tcl
Source:        %name-%version.tar.gz
URL:           https://github.com/ray2501/tcl-rocksdb
BuildRequires: autoconf
BuildRequires: make
BuildRequires: gcc-c++
BuildRequires: rocksdb-devel
BuildRequires: libstdc++-devel
BuildRequires: tcl-devel >= 8.5
Requires:      tcl >= 8.5
BuildRoot:     %{buildroot}

%description
RocksDB is a high performance embedded database for key-value data.
It is a fork of LevelDB which was then optimized to exploit many
central processing unit (CPU) cores, and make efficient use of fast
storage, such as solid-state drives (SSD), for input/output (I/O)
bound workloads. It is based on a log-structured merge-tree (LSM tree)
data structure.

This extension provides an easy to use interface for accessing RocksDB
database files from Tcl.

%prep
%setup -q -n %{name}-%{version}

%build
export CC=g++
./configure \
	--prefix=%{directory} \
	--exec-prefix=%{directory} \
	--libdir=%{directory}/%{_lib}
make 

%install
make DESTDIR=%{buildroot} pkglibdir=%{directory}/%{_lib}/tcl/%{name}%{version} install

%clean
rm -rf %buildroot

%files
%defattr(-,root,root)
%{directory}/%{_lib}/tcl
