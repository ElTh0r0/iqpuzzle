#
# spec file for iQPuzzle
#
# Copyright (C) 2012-present Thorsten Roth
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
Version:        1.4.3
Release:        1
License:        GPL-3.0+
URL:            https://github.com/ElTh0r0/iqpuzzle
Source:         %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-build

#--------------------------------------------------------------------
# Fedora
#--------------------------------------------------------------------
%if 0%{?fedora}
Group:          Amusements/Games
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib
BuildRequires:  ninja-build
%endif
#--------------------------------------------------------------------
# openSUSE
#--------------------------------------------------------------------
%if 0%{?suse_version}
Group:          Amusements/Games/Board/Puzzle
%endif
#--------------------------------------------------------------------
# All
#--------------------------------------------------------------------
BuildRequires:  gcc-c++
BuildRequires:  hicolor-icon-theme
BuildRequires:  cmake
BuildRequires:  cmake(Qt6Core)
BuildRequires:  cmake(Qt6Gui)
BuildRequires:  cmake(Qt6Widgets)
BuildRequires:  cmake(Qt6LinguistTools)
#--------------------------------------------------------------------

%description
iQPuzzle is a diverting and challenging puzzle. Pentominos are used as
jigsaw pieces and there are many different board shapes to fill with them.

%prep
%autosetup -p1

#--------------------------------------------------------------------
# Fedora
#--------------------------------------------------------------------
%if 0%{?fedora}
%build
%cmake_qt6
%cmake_build

%install
%cmake_install

%check
desktop-file-validate %{buildroot}/%{_datadir}/applications/com.github.elth0r0.iqpuzzle.desktop || :
appstream-util validate-relax --nonet %{buildroot}%{_datadir}/metainfo/com.github.elth0r0.iqpuzzle.metainfo.xml || :
%endif
#--------------------------------------------------------------------
# openSUSE
#--------------------------------------------------------------------
%if 0%{?suse_version}
%build
%cmake_qt6
%{qt6_build}

%install
%{qt6_install}
%endif
#--------------------------------------------------------------------

%files
%defattr(-,root,root,-)
%if 0%{?suse_version}
%dir %{_datadir}/metainfo
%{_datadir}/icons/hicolor/
%endif
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/com.github.elth0r0.iqpuzzle.desktop
%{_datadir}/icons/hicolor/*/apps/com.github.elth0r0.iqpuzzle.*g
%{_datadir}/metainfo/com.github.elth0r0.iqpuzzle.metainfo.xml
%doc COPYING
%{_mandir}/*/*

%changelog
