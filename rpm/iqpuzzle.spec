#
# spec file for iQPuzzle
#
# Copyright (C) 2012-2020 Thorsten Roth
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
Version:        1.2.0
Release:        1
URL:            https://github.com/ElTh0r0/iqpuzzle
Source:         %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-build

# Fedora, RHEL, or CentOS
#--------------------------------------------------------------------
%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version}
License:        GPLv3+
Group:          Amusements/Games

BuildRequires:  desktop-file-utils
BuildRequires:  gcc-c++
BuildRequires:  hicolor-icon-theme
%endif
#--------------------------------------------------------------------

# openSUSE or SLE
#--------------------------------------------------------------------
%if 0%{?suse_version}
License:        GPL-3.0+
Group:          Amusements/Games/Board/Puzzle

BuildRequires:  gcc-c++
BuildRequires:  hicolor-icon-theme
BuildRequires:  pkgconfig
BuildRequires:  update-desktop-files
Requires(post): hicolor-icon-theme
Requires(post): update-desktop-files
Requires(postun): hicolor-icon-theme
Requires(postun): update-desktop-files
%endif
#--------------------------------------------------------------------

# All
#--------------------------------------------------------------------
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5UiTools)
BuildRequires:  pkgconfig(Qt5Widgets)
#--------------------------------------------------------------------

%description
iQPuzzle is a diverting and challenging puzzle. Pentominos are used
as puzzle pieces and more than 300 different board shapes are available,
which have to be filled with them.

%prep
%setup -q -n %{name}-%{version}

# Fedora, RHEL, or CentOS
#--------------------------------------------------------------------
%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version}
%build
# Create qmake cache file to add rpm optflags.
cat > .qmake.cache <<EOF
QMAKE_CXXFLAGS += %{optflags}
EOF
%qmake_qt5 PREFIX=%{_prefix}
%make_build %{?_smp_mflags}

%install
%make_install INSTALL_ROOT=%{buildroot}

%check
desktop-file-validate %{buildroot}/%{_datadir}/%{name}.desktop || :
appstream-util validate-relax --nonet %{buildroot}%{_datadir}/res/%{name}.appdata.xml || :
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
%qmake5 PREFIX=%{_prefix}
%make_jobs %{?_smp_mflags}

%install
%qmake5_install
%suse_update_desktop_file %{name}

%if 0%{?suse_version} < 1330
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
%if 0%{?suse_version}
%dir %{_datadir}/metainfo
%{_datadir}/icons/hicolor/
%endif
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.*g
%{_datadir}/pixmaps/%{name}_64x64.png
%{_datadir}/pixmaps/%{name}.xpm
%{_datadir}/metainfo/%{name}.appdata.xml
%doc COPYING
%{_mandir}/*/*

%changelog
