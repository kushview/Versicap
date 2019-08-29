#!/usr/bin/env python

from subprocess import call, Popen, PIPE
import os, sys

sys.path.append (os.getcwd() + "/tools/waf")
import cross, versicap, juce

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

    conf.define ('VERSION_STRING', VERSION)
    conf.env.VERSION_STRING = VERSION

    print
    juce.display_header ("Versicap")
    juce.display_msg (conf, "PREFIX", conf.env.PREFIX)
    juce.display_msg (conf, "DATADIR", conf.env.DATADIR)
    juce.display_msg (conf, "Debug", conf.env.DEBUG)

    print
    juce.display_header ("Compiler")
    juce.display_msg (conf, "CFLAGS", conf.env.CFLAGS)
    juce.display_msg (conf, "CXXFLAGS", conf.env.CXXFLAGS)
    juce.display_msg (conf, "LINKFLAGS", conf.env.LINKFLAGS)

def build_sf2cute (bld):
    return bld.stlib (
        source      = bld.path.ant_glob ("libs/sf2cute/src/**/*.cpp"),
        includes    = [ 'libs/sf2cute/src', 'libs/sf2cute/include' ],
        name        = 'SF2CUTE',
        target      = 'lib/sf2cute'
    )

def build_versicap (bld):
    vcp = bld.shlib (
        source      = bld.path.ant_glob ("src/**/*.cpp") +
                      [ 'jucer/JuceLibraryCode/BinaryData.cpp',
                        'jucer/JuceLibraryCode/include_kv_core.cpp',
                        'jucer/JuceLibraryCode/include_kv_gui.cpp',
                        'jucer/JuceLibraryCode/include_kv_models.cpp',
                        'libs/libvcp/vcp.cpp' ],
        includes    = [ 'libs/compat/libjuce',
                        'libs/kv/modules',
                        'libs/jlv2/modules',
                        'libs/libvcp',
                        'libs/ksp1/src',
                        'src' ],
        target      = 'lib/vcp',
        cxxflags    = [ '-DVCP_STLIB=1' ],
        name        = 'VERSICAP',
        env         = bld.env.derive(),
        use         = [pkg.upper() for pkg in versicap.juce_packages],
        vnum        = VERSION
    )

    if bld.env.LV2:
        vcp.source.append ('jucer/JuceLibraryCode/include_jlv2_host.cpp')
        vcp.use += [ 'LV2', 'LILV', 'SUIL' ]
    
    if juce.is_mac():
        vcp.install_path = None

    bld.add_group()

    app = bld.program (
        source      = [ 'src/Main.cpp' ],
        includes    = vcp.includes,
        target      = 'bin/versicap',
        name        = 'Versicap',
        env         = bld.env.derive(),
        use         = [ 'VERSICAP' ]
    )

    if juce.is_mac():
        app.target       = 'Applications/Versicap'
        app.mac_app      = True,
        app.mac_plist    = 'tools/macdeploy/Info.plist',
        app.mac_files    = [ 'tools/macdeploy/Icon.icns' ]

    bld.add_group()

    tests = bld.program (
        source          = bld.path.ant_glob ("tests/**/*.cpp"),
        includes        = vcp.includes,
        target          = 'bin/test-versicap',
        name            = 'TEST_VERSICAP',
        env             = bld.env.derive(),
        use             = [ 'VERSICAP' ],
        install_path    = None
    )

    return (vcp, app, tests)

def build_plugin (bld, slug):
    plugin = bld.shlib (
        source      = [ 'plugins/%s.vcp/%s.cpp' % (slug, slug) ],
        includes    = [ 'libs/libvcp' ],
        cxxflags    = [ '-fvisibility=hidden' ],
        target      = 'plugins/%s.vcp/%s' % (slug, slug),
        name        = '%s_PLUGIN' % slug.upper(),
        env         = bld.env.derive(),
        use         = [ 'VERSICAP' ]
    )
    plugin.env.cxxshlib_PATTERN = plugin.env.plugin_PATTERN
    return plugin

def build (bld):
    build_versicap (bld)
    for plugin in 'test'.split():
        build_plugin (bld, plugin)

def macdeploy (ctx):
    call (["tools/macdeploy/appbundle.py",
           "-verbose", "2",
           "-dmg", "versicap-osx-%s" % VERSION,
           "-volname", "Versicap",
           "-fancy", "tools/macdeploy/fancy.plist",
           "build/Applications/Versicap.app"])

def macrelease (ctx):
    call (["bash", "tools/macdeploy/sync-jucer.sh"])
    call (["bash", "tools/macdeploy/clean.sh"])
    call (["python", "waf", "distclean", "configure", "build", "macdeploy"])
