#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os, platform
from waflib.Configure import conf

juce_modules = '''
    juce_analytics juce_audio_basics juce_audio_devices juce_audio_formats
    juce_audio_processors juce_audio_utils juce_core juce_cryptography
    juce_data_structures juce_events juce_graphics juce_gui_basics
    juce_gui_extra juce_product_unlocking kv_core kv_edd kv_engines
    kv_gui kv_lv2 kv_models
'''

juce_packages = '''
    juce_cryptography
    juce_audio_devices
    juce_audio_formats
    juce_audio_processors
    juce_audio_utils'''.split()

mingw_libs = '''
    uuid wsock32 wininet version ole32 ws2_32 oleaut32
    imm32 comdlg32 shlwapi rpcrt4 winmm gdi32
'''

@conf 
def check_common (self):
    self.check(header_name='stdbool.h', mandatory=True)
    self.check_cfg (package='lv2', uselib_store='LV2', args=['--cflags'], mandtory=False)
    self.check_cfg (package='lilv-0', uselib_store='LILV', args=['--cflags', '--libs'], mandatory=False)
    self.check_cfg (package='suil-0', uselib_store='SUIL', args=['--cflags', '--libs'], mandatory=False)
    self.env.LV2 = bool(self.env.HAVE_LV2) and bool(self.env.HAVE_LILV) and bool(self.env.HAVE_SUIL)

    for jpkg in juce_packages:
        name = '%s-5' % jpkg if not self.options.debug else '%s_debug-5' % jpkg
        self.check_cfg (package=name, uselib_store=jpkg.upper(), mandatory=True,
                        args=['--cflags', '--libs'])

@conf
def check_xmingw (self):
    for l in mingw_libs.split():
        self.check_cxx(lib=l, uselib_store=l.upper())

@conf
def check_mac (self):
    # Python scripting
    if (self.options.enable_python):
        self.check_cfg(path='python-config', args='--includes', package='Python', use='PYTHON')

@conf
def check_linux (self):
    return

def get_mingw_libs():
    return [ l.upper() for l in mingw_libs.split() ]

def get_juce_library_code (prefix, extension='.cpp'):
    cpp_only = [ 'juce_analytics' ]
    code = []
    for f in juce_modules.split():
        e = '.cpp' if f in cpp_only else extension
        code.append (prefix + '/include_' + f + e)
    return code
