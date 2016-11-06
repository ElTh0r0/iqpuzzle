#
# spec file for iQPuzzle
#
# Copyright (C) 2012-2016 Thorsten Roth
#
# iQPuzzle is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# iQPuzzle is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with iQPuzzle.  If not, see <http://www.gnu.org/licenses/>.

Name:           iqpuzzle
Summary:        Challenging pentomino puzzle
Version:        1.0.2
Release:        1
License:        GPL-3.0+
URL:            https://launchpad.net/iqpuzzle
Source:         %{name}-%{version}-src.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-build

# Fedora, RHEL, or CentOS
#--------------------------------------------------------------------
%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version}
Group:          Amusements/Games
BuildRequires:  desktop-file-utils
BuildRequires:  gcc-c++
BuildRequires:  hicolor-icon-theme
%if 0%{?fedora} >= 19
BuildRequires:  qt5-qtbase-devel
%else
BuildRequires:  qt-devel >= 4.5
%endif
%endif
#--------------------------------------------------------------------

# openSUSE or SLE
#--------------------------------------------------------------------
%if 0%{?suse_version}
Group:          Amusements/Games/Board/Puzzle
BuildRequires:  gcc-c++
BuildRequires:  hicolor-icon-theme
%if 0%{?suse_version} >= 1310
BuildRequires:  libqt5-qtbase-devel
%else
BuildRequires:  libqt4-devel >= 4.5
%endif
BuildRequires:  update-desktop-files
%endif
#--------------------------------------------------------------------

%description
iQPuzzle is a diverting and challenging puzzle. Pentominos are used
as puzzle pieces and 195 different board shapes are available, which
have to be filled with them.

%prep
%setup -q -n %{name}-%{version}-src

# Fedora, RHEL, or CentOS
#--------------------------------------------------------------------
%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version}
%build
# Create qmake cache file to add rpm optflags.
cat > .qmake.cache <<EOF
QMAKE_CXXFLAGS += %{optflags}
EOF
%if 0%{?fedora} >= 19
%{qmake_qt5} PREFIX=%{_prefix}
%else
qmake-qt4 PREFIX=%{_prefix}
%endif
make %{?_smp_mflags}

%install
make install INSTALL_ROOT=%{buildroot}
desktop-file-validate %{buildroot}/%{_datadir}/applications/%{name}.desktop

%clean
rm -rf %{buildroot}

%post
update-desktop-database &> /dev/null || :
touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :

%postun
update-desktop-database &> /dev/null || :
if [ $1 -eq 0 ] ; then
    touch --no-create %{_datadir}/icons/hicolor &>/dev/null
    gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi

%posttrans
gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
%endif
#--------------------------------------------------------------------

# openSUSE or SLE
#--------------------------------------------------------------------
%if 0%{?suse_version}
%build
# Create qmake cache file to add rpm optflags.
cat > .qmake.cache <<EOF
QMAKE_CXXFLAGS += %{optflags}
EOF
%if 0%{?suse_version} >= 1310
qmake-qt5 PREFIX=%{_prefix}
%else
qmake PREFIX=%{_prefix}
%endif
make %{?_smp_mflags}

%install
make INSTALL_ROOT=%{buildroot} install
%suse_update_desktop_file %{name}

%clean
rm -rf %{buildroot}

%if 0%{?suse_version} >= 1140
%post
%desktop_database_post
%icon_theme_cache_post

%postun
%desktop_database_postun
%icon_theme_cache_postun
%endif
%endif
#--------------------------------------------------------------------

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/*/%{name}.*g
%{_datadir}/pixmaps/%{name}_64x64.png
%{_datadir}/pixmaps/%{name}.xpm
%doc COPYING
%{_mandir}/*/*

%changelog
