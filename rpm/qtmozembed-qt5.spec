Name:       qtmozembed-qt5
Summary:    Qt embeddings for Gecko
Version:    1.2.3
Release:    1
Group:      Applications/Internet
License:    Mozilla License
URL:        https://github.com/tmeshkova/qtmozembed.git
Source0:    %{name}-%{version}.tar.bz2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5OpenGL)
BuildRequires:  pkgconfig(Qt5Widgets)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Declarative)
BuildRequires:  pkgconfig(Qt5QuickTest)
BuildRequires:  xulrunner-qt5-devel
BuildRequires:  pkgconfig(nspr)
BuildRequires:  qt5-default

%description
Qt embeddings for Gecko browser engine

%package devel
Group:      Applications/Internet
Requires:   %{name} = %{version}-%{release}
Summary:    Headers for qtmozembed

%description devel
Development files for qtmozembed.

%package tests
Summary:    Unit tests for QtMozEmbed tests
Group:      Applications/Internet
Requires:   %{name} = %{version}-%{release}
Requires:   embedlite-components-qt5 >= 1.0.10

%description tests
This package contains QML unit tests for QtMozEmbed library

%prep
%setup -q -n %{name}-%{version}

%build
qmake
%{__make} %{?jobs:MOZ_MAKE_FLAGS="-j%jobs"}

%install
%{__make} install INSTALL_ROOT=%{buildroot}
%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/*.so.*
%{_libdir}/qt4/imports/*
%{_libdir}/qt5/imports/*

%files devel
%defattr(-,root,root,-)
%{_libdir}/*.so
%{_libdir}/pkgconfig
%{_includedir}/*

%files tests
%defattr(-,root,root,-)
# >> files tests
/opt/tests/qtmozembed/*
%{_libdir}/qt5/bin/*
# << files tests
