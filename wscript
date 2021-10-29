# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('ns-3-runtime-test', ['core'])
    module.source = [
        'model/ns-3-runtime-test.cc',
        'helper/ns-3-runtime-test-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('ns-3-runtime-test')
    module_test.source = [
        'test/ns-3-runtime-test-test-suite.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'ns-3-runtime-test'
    headers.source = [
        'model/ns-3-runtime-test.h',
        'helper/ns-3-runtime-test-helper.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()

