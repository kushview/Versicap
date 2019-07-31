#!/usr/bin/env python

from subprocess import call, Popen, PIPE
import os, sys

sys.path.append (os.getcwd() + "/tools/waf")
import cross, element, juce

VERSION='1.0.0b1'

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
    return "Versicap"

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
    if conf.env.DEBUG:
        conf.define ('JUCE_DISABLE_AUDIOPROCESSOR_BEGIN_END_GESTURE_CHECKING', 1)

    conf.env.VERSION_STRING = VERSION

    configure_product (conf)

    conf.define ('VERSION_STRING', conf.env.VERSION_STRING)

    conf.check_cfg (package='kv_debug-0' if conf.options.debug else 'kv-0',
                    uselib_store='KV', args=['--libs', '--cflags'], 
                    mandatory=True)

    print
    juce.display_header ("Versicap Configuration")
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

def build_sf2cute (bld):
    return bld.stlib (
        source = bld.path.ant_glob ("libs/sf2cute/src/**/*.cpp"),
        includes = [ 'libs/sf2cute/src', \
                     'libs/sf2cute/include' ],
        name   = 'SF2CUTE',
        target  = 'lib/sf2cute'
    )

def build_mac (bld):
    bld.add_group()
    libEnv = bld.env.derive()
    versicap = bld.shlib (
        source      = bld.path.ant_glob ("src/**/*.cpp") +
                      [ 'jucer/JuceLibraryCode/BinaryData.cpp' ],
        includes    = [ 'libs/compat/libjuce', 'libs/libkv', 'libs/ksp1/src', 'src' ],
        target      = 'lib/vcp',
        cxxflags    = [ '-DVCP_STLIB=1' ],
        name        = 'VERSICAP',
        env         = libEnv,
        use         = [ 'KV' ]
    )

    bld.add_group()

    appEnv = bld.env.derive()
    bld.program (
        source      = [ 'src/Main.cpp' ],
        includes    = versicap.includes,
        target      = 'Applications/Versicap',
        name        = 'Versicap',
        env         = appEnv,
        use         = [ 'VERSICAP' ],
        mac_app     = True,
        mac_plist   = 'tools/macdeploy/Info.plist',
        mac_files   = [ 'tools/macdeploy/Icon.icns' ]
    )

    bld.add_group()
    bld.program (
        source      = bld.path.ant_glob ("tests/**/*.cpp"),
        includes    = versicap.includes,
        target      = 'bin/test-versicap',
        name        = 'TEST_VERSICAP',
        env         = appEnv,
        use         = [ 'VERSICAP' ]
    )

def build_plugin (bld, slug):
    pluginEnv = bld.env.derive()
    pluginEnv.cxxshlib_PATTERN = pluginEnv.plugin_PATTERN
    bld.shlib (
        source      = [ 'plugins/%s.vcp/%s.cpp' % (slug, slug) ],
        includes    = [ 'libs/versicap', 'libs/libkv' ],
        target      = 'plugins/%s.vcp/%s' % (slug, slug),
        name        = '%s_PLUGIN' % slug.upper(),
        env         = pluginEnv,
        use         = [ 'VERSICAP' ]
    )

def build (bld):
    build_sf2cute (bld)
    if juce.is_mac():   build_mac (bld)
    for plugin in 'test'.split():
        build_plugin (bld, plugin)

def macdeploy (ctx):
    call (["tools/macdeploy/appbundle.py",
           "-verbose", "3",
           "-dmg", "versicap-osx-%s" % VERSION,
           "-volname", "Versicap",
           "-fancy", "tools/macdeploy/fancy.plist",
           "build/Applications/Versicap.app"])

def macrelease (ctx):
    call (["bash", "tools/macdeploy/sync-jucer.sh"])
    call (["bash", "tools/macdeploy/clean.sh"])
    call (["python", "waf", "distclean", "configure", "build", "macdeploy"])
