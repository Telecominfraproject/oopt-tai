#
# Copyright (C) 2018 Nippon Telegraph and Telephone Corporation.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys

import clang.cindex
from clang.cindex import Index
from clang.cindex import Config
from optparse import OptionParser
from enum import Enum
from jinja2 import Environment


class TAIAttributeFlag(Enum):
    READ_ONLY = 0
    IS_FLAG = 1
    CREATE_ONLY = 2
    MANDATORY_ON_CREATE = 3


class TAIEnum(object):
    def __init__(self, name_node, value_nodes):
        self.name_node = name_node
        value_nodes.sort(key = lambda l : l.enum_value)
        self.value_nodes = value_nodes
        # displayname starts with '_'. remove it
        self.typename = self.name_node.displayname[1:]

    def value_names(self):
        return [v.displayname for v in self.value_nodes if not v.displayname.endswith('_START') and not v.displayname.endswith('_END')]


class TAIAttribute(object):
    def __init__(self, node, taiobject):
        self.node = node
        self.id = node.enum_value
        self.name = node.displayname
        self.taiobject = taiobject
        self.object_type = taiobject.object_type
        self.object_name = taiobject.name

        rm = ' /*'
        cmt = [l.strip(rm).split(' ') for l in node.raw_comment.split('\n') if l.strip(rm).startswith('@')] # this omits long description from the comment
        s = { l[0][1:]: ' '.join(l[1:]) for l in cmt }
        self.cmt = s
        flags = self.cmt.get('flags', '').split('|')
        if flags[0] != '':
            self.flags = set(TAIAttributeFlag[e.strip()] for e in flags)
        else:
            self.flags = None
        t = self.cmt['type']
        ts = [v.strip('#') for v in t.split(' ')]
        self.enum_type = None
        if len(ts) == 1:
            self.type = ts[0]
        elif len(ts) == 2:
            if ts[0] == 'tai_s32_list_t':
                self.type = ts[0]
                self.enum_type = ts[1]
            elif ts[0] == 'tai_pointer_t':
                self.type = ts[0]
            else:
                raise Exception("unsupported type format: {}".format(t))
        else:
            raise Exception("unsupported type format: {}".format(t))

    def __str__(self):
        return self.name

    def __repr__(self):
        return '{}[{}:{}]'.format(self.name, self.type, self.flags)


class TAIObject(object):
    OBJECT_MAP = {
        'module'           : 'TAI_OBJECT_TYPE_MODULE',
        'host_interface'   : 'TAI_OBJECT_TYPE_HOSTIF',
        'network_interface': 'TAI_OBJECT_TYPE_NETWORKIF',
    }

    def __init__(self, name, header):
        self.name = name
        index = Index.create()
        tu = index.parse(header)
        self.tu = tu
        self.object_type = self.OBJECT_MAP.get(name, None)

        self.enum_map = {}

        m = {n.displayname:n for n in self.kinds('ENUM_DECL')}
        n = {n.displayname:n for n in self.kinds('ENUM_CONSTANT_DECL')}

        for k, v in m.items():
            prefix = k[1:-2].upper()
            l = [vv for vv in n.values() if vv.displayname.startswith(prefix)]
            e = TAIEnum(v, l)
            self.enum_map[e.typename] = e

        v = self.get_name('_tai_attribute_value_t')
        m = {}
        for f in v.get_children():
            # bool needs special handling since its type.spelling appear as 'int'
            key = 'bool' if f.displayname == 'booldata' else f.type.spelling
            m[key] = f.displayname

        enums = set()

        a = self.get_enum('tai_{}_attr_t'.format(self.name))
        attrs = [ TAIAttribute(e, self) for e in a.value_nodes if not e.displayname.endswith('_START') and not e.displayname.endswith('_END') ]

        for attr in attrs:
            field = m.get(attr.type, None)
            if field:
                if getattr(attr, 'enum_type', None):
                    enum = self.get_enum(attr.enum_type)
                    if not enum:
                        raise Exception("{} not found".format(attr.enum_type))
                    enums.add(attr.enum_type)
                attr.value_field = field
            else:
                enum = self.get_enum(attr.type)
                enums.add(attr.type)
                if not enum:
                    raise Exception("{} not found".format(attr.type))
                attr.value_field = 's32'
                attr.enum_type = attr.type

        self.enum_names = enums
        self.attrs = attrs

    def kinds(self, kind):
        node = self.tu.cursor
        out = []
        self.get_kinds(node, kind, out)
        return out

    def get_name(self, name):
        node = self.tu.cursor
        return self._get_name(node, name)

    def _get_name(self, node, name):
        if node.displayname == name:        
            return node
        for child in node.get_children():
            n = self._get_name(child, name)
            if n:
                return n
        return None

    def get_kinds(self, node, kind, out=[]):
        if node.kind.name == kind:
            out.append(node)
            return
        for child in node.get_children():
            self.get_kinds(child, kind, out)

    def get_enum(self, name):
        return self.enum_map.get(name, None)

    def get_enums(self):
        return [self.enum_map[n] for n in self.enum_names]

    def get_attributes(self):
        return self.attrs


class Generator(object):
    HEADER_TEMPLATE = ''

    def __init__(self, env=Environment()):
        self.env = env

    def implementation(self):
        if not getattr(self, 'env', None):
            self.env = Environment()
        return self.env.from_string(self.IMPL_TEMPLATE).render(self.data)

    def header(self):
        if not getattr(self, 'env', None):
            self.env = Environment()
        return self.env.from_string(self.HEADER_TEMPLATE).render(self.data)


class ObjectMetadataGenerator(Generator):
    HEADER_TEMPLATE = '''
extern const tai_object_type_info_t tai_metadata_object_type_info_{{ name }};
'''

    IMPL_TEMPLATE = '''
const tai_attr_metadata_t* const tai_metadata_object_type_tai_{{ name }}_attr_t[] = {
    {% for a in attrs -%}
    &tai_metadata_attr_{{ a }},
    {% endfor -%}
    NULL
};

const tai_object_type_info_t tai_metadata_object_type_info_{{ name }} = {
    .objecttype         = {{ object_type }},
    .objecttypename     = "{{ object_type }}",
    .enummetadata       = &tai_metadata_enum_tai_{{ name }}_attr_t,
    .attrmetadata       = tai_metadata_object_type_tai_{{ name }}_attr_t,
    .attrmetadatalength = {{ attrs | count }},
};
'''

    def __init__(self, obj):
        self.data = {'name': obj.name,
                     'object_type' : obj.object_type,
                     'attrs': [a.name for a in obj.get_attributes()]}


class AttrMetadataGenerator(Generator):
    IMPL_TEMPLATE = '''
const tai_attr_metadata_t tai_metadata_attr_{{ typename }} = {
    .objecttype      = {{ object }},
    .attrid          = {{ typename }},
    .attridname      = "{{ typename }}",
    .attridshortname = "{{ shorttypename }}",
    .attrvaluetype   = {{ attr_type }},
{%- if attr_flags %}
    .flags           = {{ attr_flags }},
{%- else %}
    .flags           = 0,
{%- endif %}
    .isenum          = {{ is_enum }},
{%- if enum_meta_data %}
    .enummetadata    = &{{ enum_meta_data }},
{%- else %}
    .enummetadata    = NULL,
{%- endif %}
};
'''

    def __init__(self, attr):
        obj = attr.object_type
        objname = attr.object_name
        typename = attr.name
        prefix = 'TAI_{}_ATTR_'.format(objname.upper())
        if not typename.startswith(prefix):
            raise Exception("invalid attr type name: {}, obj: {}".format(typename, obj))
        shorttypename = typename[len(prefix):].lower().replace('_', '-')
        attr_type = 'TAI_ATTR_VALUE_TYPE_{}'.format(attr.value_field.upper())
        is_enum = 'false'
        enum_meta_data = None
        if attr.enum_type:
            is_enum = 'true'
            enum_meta_data = 'tai_metadata_enum_{}'.format(attr.enum_type)
        attr_flags = None
        if attr.flags:
            attr_flags = '|'.join('TAI_ATTR_FLAGS_{}'.format(e.name) for e in list(attr.flags))

        self.data = {'object': obj,
                     'typename': typename,
                     'shorttypename': shorttypename,
                     'attr_type': attr_type,
                     'attr_flags': attr_flags,
                     'is_enum': is_enum,
                     'enum_meta_data': enum_meta_data}


class EnumMetadataGenerator(Generator):
    HEADER_TEMPLATE = '''int tai_serialize_{{ typename | simplify }}( _Out_ char *buffer, _In_ {{ typename }} {{ typename | simplify }}, _In_ const tai_serialize_option_t *option);;
int tai_deserialize_{{ typename | simplify }}( _In_ const char *buffer, _Out_ int32_t *value, _In_ const tai_serialize_option_t *option);
'''


    IMPL_TEMPLATE = '''const {{ typename }} tai_metadata_{{ typename }}_enum_values[] = {
    {% for t in enums -%}
    {{ t }},
    {% endfor -%}
    -1
};

const char* const tai_metadata_{{ typename }}_enum_values_names[] = {
    {% for t in enums -%}
    "{{ t }}",
    {% endfor -%}
    NULL
};

const char* const tai_metadata_{{ typename }}_enum_values_short_names[] = {
    {% for t in enums -%}
    "{{ t | shorten(typename) }}",
    {% endfor -%}
    NULL
};

const tai_enum_metadata_t tai_metadata_enum_{{ typename }} = {
    .name             = "{{ typename }}",
    .valuescount      = {{ enums | count }},
    .values           = (const int*)tai_metadata_{{ typename }}_enum_values,
    .valuesnames       = tai_metadata_{{ typename }}_enum_values_names,
    .valuesshortnames  = tai_metadata_{{ typename }}_enum_values_short_names,
    .containsflags    = false,
};

int tai_serialize_{{ typename | simplify }}(
    _Out_ char *buffer,
    _In_ {{ typename }} {{ typename | simplify }},
    _In_ const tai_serialize_option_t *option)
{
    return tai_serialize_enum(buffer, &tai_metadata_enum_{{ typename }}, {{ typename | simplify }}, option);
}

int tai_deserialize_{{ typename | simplify }}(
    _In_ const char *buffer,
    _Out_ int32_t *value,
    _In_ const tai_serialize_option_t *option)
{
    return tai_deserialize_enum(buffer, &tai_metadata_enum_{{ typename }}, value, option);
}
'''

    def __init__(self, enum):
        typename = enum.typename
        enums = enum.value_names()

        env = Environment()
        def shorten(v, typename):
            if not typename.endswith('_t'):
                raise Exception("invalid type name: {}".format(typename))
            t = typename[:-1].upper()
            if not v.startswith(t):
                raise Exception("invalid enum value name: {}".format(v))
            return v[len(t):].lower().replace('_', '-')

        def simplify(typename):
            return typename[4:-2]
        env.filters['shorten'] = shorten
        env.filters['simplify'] = simplify
        super(EnumMetadataGenerator, self).__init__(env)
        self.data = {'typename': typename, 'enums': enums}


class TAIMetadataGenerator(Generator):
    HEADER_TEMPLATE = '''/* AUTOGENERATED FILE! DO NOT EDIT */
#ifndef __TAI_METADATA_H__
#define __TAI_METADATA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <tai.h>
#include "taimetadatatypes.h"
#include "taimetadatautils.h"
#include "taimetadatalogger.h"
#include "taiserialize.h"

{% for e in headers -%}
{{ e }}
{% endfor -%}

extern const tai_attr_metadata_t* const* const tai_metadata_attr_by_object_type[];
extern const size_t tai_metadata_attr_by_object_type_count;

/* Object infos table */

extern const tai_object_type_info_t* const tai_metadata_all_object_type_infos[];

/* List of all attributes */

extern const tai_attr_metadata_t* const tai_metadata_attr_sorted_by_id_name[];
extern const size_t tai_metadata_attr_sorted_by_id_name_count;

#ifdef __cplusplus
}
#endif

#endif'''

    IMPL_TEMPLATE = '''/* AUTOGENERATED FILE! DO NOT EDIT */
#include <stdio.h>
#include "taimetadata.h"

{% for e in impls -%}
{{ e }}
{% endfor -%}

volatile tai_log_level_t tai_metadata_log_level = TAI_LOG_LEVEL_NOTICE;
volatile tai_metadata_log_fn tai_metadata_log = NULL;

const tai_attr_metadata_t* const* const tai_metadata_attr_by_object_type[] = {
    NULL,
    {% for o in object_names -%}
    tai_metadata_object_type_tai_{{ o }}_attr_t,
    {% endfor -%}
    NULL
};
const size_t tai_metadata_attr_by_object_type_count = {{ object_names | count }};

const tai_object_type_info_t* const tai_metadata_all_object_type_infos[] = {
    NULL,
    {% for o in object_names -%}
    &tai_metadata_object_type_info_{{ o }},
    {% endfor -%}
    NULL
};

const tai_attr_metadata_t* const tai_metadata_attr_sorted_by_id_name[] = {
    {% for a in attrs -%}
    &tai_metadata_attr_{{ a }},
    {% endfor -%}
    NULL
};

const size_t tai_metadata_attr_sorted_by_id_name_count = {{ attrs | count }};

'''

    def __init__(self, objects):
        generators = []
        all_attrs = []
        for obj in objects:
            attrs = obj.get_attributes()
            enums = obj.get_enums()

            for e in enums:
                g = EnumMetadataGenerator(e)
                generators.append(g)

            for a in attrs:
                g = AttrMetadataGenerator(a)
                generators.append(g)

            e = obj.get_enum('tai_{}_attr_t'.format(obj.name))
            g = EnumMetadataGenerator(e)
            generators.append(g)

            g = ObjectMetadataGenerator(obj)
            generators.append(g)

            all_attrs += [a.name for a in attrs]

        self.data = {'impls': [g.implementation() for g in generators],
                     'headers': [x for x in [g.header() for g in generators] if len(x) > 0],
                     'object_names': [o.name for o in objects],
                     'attrs': sorted(all_attrs)}


if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option('--clang-lib', default='/usr/lib/llvm-6.0/lib/libclang.so.1')
    (options, args) = parser.parse_args()

    Config.set_library_file(options.clang_lib)

    objects = [TAIObject(t, args[0]) for t in TAIObject.OBJECT_MAP.keys()]

    g = TAIMetadataGenerator(objects)
    with open('taimetadata.h', 'w') as f:
        f.write(g.header())

    with open('taimetadata.c', 'w') as f:
        f.write(g.implementation())
