## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    obj = bld.create_ns3_program('my_openflow', ['core', 'internet', 'applications'])
    obj.source = 'my_openflow.cc'
        
    obj = bld.create_ns3_program('my_test', ['core', 'test', 'internet', 'applications'])
    obj.source = 'my_test.cc'

    obj = bld.create_ns3_program('other_ones_of', ['core', 'stat', 'csma', 'internet', 'applications'])
    obj.source = 'other_ones_of.cc'
