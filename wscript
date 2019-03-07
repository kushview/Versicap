#!/usr/bin/env python

from subprocess import call, Popen, PIPE
import os, sys

sys.path.append (os.getcwd() + "/tools/waf")
import cross, element, juce

VERSION='1.0.0'

def options (opt):
    opt.load ("compiler_c compiler_cxx cross juce")
    opt.add_option ('--disable-unlocking', default=False, action="store_true", dest="disable_unlocking", \
        help="Build without license protection [ Default: False ]")
    opt.add_option ('--disable-unlocking', default=False, action="store_true", dest="disable_unlocking", \
        help="Build without license protection [ Default: False ]")
    opt.add_option ('--enable-free', default=False, action='store_true', dest='enable_free', \
        help="Build Element Lite")
    opt.add_option ('--enable-solo', default=False, action='store_true', dest='enable_solo', \
        help="Build Element Solo")
    opt.add_option ('--enable-docking', default=False, action='store_true', dest='enable_docking', \
        help="Build with docking window support")
    opt.add_option ('--enable-local-auth', default=False, action='store_true', dest='enable_local_auth', \
        help="Authenticate locally")
    opt.add_option ('--enable-python', default=False, action='store_true', dest='enable_python', \
        help="Enable Python scripting support")

def silence_warnings (conf):
    '''TODO: resolve these'''
    conf.env.append_unique ('CFLAGS', ['-Wno-deprecated-register'])
    conf.env.append_unique ('CXXFLAGS', ['-Wno-deprecated-register'])
    conf.env.append_unique ('CFLAGS', ['-Wno-dynamic-class-memaccess'])
    conf.env.append_unique ('CXXFLAGS', ['-Wno-dynamic-class-memaccess'])
    conf.env.append_unique ('CFLAGS', ['-Wno-deprecated-declarations'])
    conf.env.append_unique ('CXXFLAGS', ['-Wno-deprecated-declarations'])

def configure_product (conf):
    # noop
    return

def configure_product_name (conf):
    return "Versacap"

def configure (conf):
    conf.check_ccache()
    cross.setup_compiler (conf)
    if len(conf.options.cross) <= 0:
        conf.prefer_clang()
    conf.load ("compiler_c compiler_cxx ar cross juce")
    conf.check_cxx_version()

    silence_warnings (conf)

    conf.check_common()
    if cross.is_mingw(conf): conf.check_mingw()
    elif juce.is_mac(): conf.check_mac()
    else: conf.check_linux()

    conf.env.DEBUG = conf.options.debug
    conf.env.VERSION_STRING = VERSION

    configure_product (conf)

    conf.define ('VERSION_STRING', conf.env.VERSION_STRING)

    conf.check(lib='curl', mandatory=False)
    if juce.is_linux():
        conf.check(lib='pthread', mandatory=True)
        conf.check(lib='dl', mandatory=True)
        conf.check_cfg(package='freetype2', args='--cflags --libs', \
            mandatory=True)
        conf.check_cfg(package='x11', args='--cflags --libs', \
            mandatory=True)
        conf.check_cfg(package='xext', args='--cflags --libs', \
            mandatory=True)
        conf.check_cfg(package='alsa', args='--cflags --libs', \
            mandatory=True)
    if cross.is_windows (conf):
        conf.check(lib='ws2_32', mandatory=True)
        conf.check(lib='pthread', mandatory=True)

    print
    juce.display_header ("Element Configuration")
    juce.display_msg (conf, "Product", configure_product_name (conf))
    juce.display_msg (conf, "Docking Windows", conf.options.enable_docking)
    juce.display_msg (conf, "Copy Protection", not conf.options.disable_unlocking)
    juce.display_msg (conf, "Local authentication", conf.options.enable_local_auth)
    print
    juce.display_msg (conf, "Installation PREFIX", conf.env.PREFIX)
    juce.display_msg (conf, "Installation DATADIR", conf.env.DATADIR)
    juce.display_msg (conf, "Debugging Symbols", conf.options.debug)
    print
    juce.display_header ("Compiler")
    juce.display_msg (conf, "CFLAGS", conf.env.CFLAGS)
    juce.display_msg (conf, "CXXFLAGS", conf.env.CXXFLAGS)
    juce.display_msg (conf, "LINKFLAGS", conf.env.LINKFLAGS)

def common_use_flags():
    return 'ACCELERATE AUDIO_TOOLBOX AUDIO_UNIT CORE_AUDIO CORE_AUDIO_KIT COCOA CORE_MIDI IO_KIT QUARTZ_CORE'.split()
def build_mac (bld):
    appEnv = bld.env.derive()
    app = bld.program (
        source      = bld.path.ant_glob ("src/**/*.cpp") +
                      bld.path.ant_glob ("jucer/JuceLibraryCode/*.mm"),
        includes    = [ 'jucer/JuceLibraryCode', 'src', \
                        os.path.expanduser('~') + '/SDKs/VST_SDK/VST3_SDK', \
                        os.path.expanduser('~') + '/SDKs/JUCE/modules' ],
        target      = 'Applications/Versacap',
        name        = 'Versacap',
        env         = appEnv,
        use         = common_use_flags(),
        mac_app     = True #,
        # mac_plist   = 'tools/macdeploy/Info.plist',
        # mac_files   = [ 'project/Builds/MacOSX/Icon.icns' ]
    )
    
def build (bld):
    build_mac (bld)
