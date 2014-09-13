from apport.hookutils import *
import os

def add_info(report, ui=None):
    if not apport.packaging.is_distro_package(report['Package'].split()[0]):
        report['ThirdParty'] = 'True'
        report['CrashDB'] = 'iqpuzzle'
        attach_file_if_exists(report, os.getenv("HOME") + '/.local/share/data/iqpuzzle/debug.log', key='debug.log')
