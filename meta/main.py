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
    MANDATORY_ON_CREATE = 0
    CREATE_ONLY         = 1
    CREATE_AND_SET      = 2
    READ_ONLY           = 3
    KEY                 = 4
    DYNAMIC             = 5
    SPECIAL             = 6

class TAIDefaultValueType(Enum):
    NONE = 0
    CONST = 1
    RANGE = 2
    VALUE = 3
    EMPTY_LIST = 4
    VENDOR_SPECIFIC = 5


def process_type(header, type_):
    ts = [v.strip('#') for v in type_.split(' ')]
    t = e = v = attrlistvaluetype  = None
    if len(ts) == 1:
        t = ts[0]
    elif len(ts) == 2:
        if ts[0] == 'tai_s32_list_t':
            t = ts[0]
            e = ts[1]
        elif ts[0] == 'tai_pointer_t':
            t = ts[0]
        elif ts[0] == 'tai_attr_value_list_t':
            t = ts[0]
            if ts[1] == 'tai_attr_value_list_t':
                raise Exception("unsupported type format: {}".format(type_))
            attrlistvaluetype = ts[1]
        else:
            raise Exception("unsupported type format: {}".format(type_))
    elif len(ts) == 3:
        if ts[0] == 'tai_attr_value_list_t':
            t = ts[0]
            if ts[1] == 'tai_attr_value_list_t':
                raise Exception("unsupported type format: {}".format(type_))
            attrlistvaluetype = ts[1]
            e = ts[2]
        else:
            raise Exception("unsupported type format: {}".format(type_))
    else:
        raise Exception("unsupported type format: {}".format(type_))

    field = header.attr_value_map.get(t, None)
    # if we can't find type name in attr_value_map, the type should be enum type
    if field:
        v = field
    else:
        e = t
        v = 's32'

    if attrlistvaluetype:
        attrlistvaluetype = header.attr_value_map.get(attrlistvaluetype, None)

    if e and not header.get_enum(e):
        raise Exception("{} not found".format(e))

    return t, e, v, attrlistvaluetype


def process_default_value_type(default):
    if default == '':
        return TAIDefaultValueType.NONE

    if default in ['NULL', 'true', 'false'] or default.isdigit():
        return TAIDefaultValueType.CONST

    try:
        default = default.upper().replace('-', '_').strip()
        return TAIDefaultValueType[default]
    except KeyError:
        pass

    if default.startswith('TAI_'):
        return TAIDefaultValueType.CONST

    raise Exception('invalid default value: {}'.format(default))


class TAIEnum(object):
    def __init__(self, node, exclude_range_indicator=True):
        self.name_node = node
        value_nodes = list(node.get_children())
        value_nodes.sort(key = lambda l : l.enum_value)
        self.value_nodes = value_nodes
        if exclude_range_indicator:
            self.value_nodes = [v for v in self.value_nodes if not v.displayname.endswith('_START') and not v.displayname.endswith('_END')]
        # displayname starts with '_'. remove it
        self.typename = self.name_node.displayname[1:]

    def value_names(self):
        return [v.displayname for v in self.value_nodes]

class TAIAttribute(object):
    def __init__(self, node, taiobject):
        self.node = node
        self.id = node.enum_value
        self.name = node.displayname
        self.taiobject = taiobject
        self.object_type = taiobject.object_type
        self.object_name = taiobject.name

        rm = ' /*'
        if node.raw_comment is None:
            raise Exception("no comment detected for the attribute: {}".format(node.displayname))

        cmt = [l.strip(rm).split(' ') for l in node.raw_comment.split('\n') if l.strip(rm).startswith('@')] # this omits long description from the comment
        s = { l[0][1:]: ' '.join(l[1:]) for l in cmt }
        self.cmt = s
        # process flags command
        flags = self.cmt.get('flags', '').split('|')
        if flags[0] != '':
            self.flags = set(TAIAttributeFlag[e.strip()] for e in flags)
        else:
            self.flags = None
        # process type command
        t = self.cmt['type']
        self.type, self.enum_type, self.value_field, self.attrlist_value_type = process_type(self.taiobject.taiheader, t)

        # process default command
        self.default = self.cmt.get('default', '')
        self.default_type = process_default_value_type(self.default)

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

    def __init__(self, name, taiheader):
        self.name = name
        self.taiheader = taiheader
        self.object_type = self.OBJECT_MAP.get(name, None)
        a = self.taiheader.get_enum('tai_{}_attr_t'.format(self.name))
        self.attrs = [ TAIAttribute(e, self) for e in a.value_nodes ]

    def get_attributes(self):
        return self.attrs

    def get_enums(self):
        return set(self.taiheader.enum_map[a.enum_type] for a in self.attrs if a.enum_type)


class TAIHeader(object):
    def __init__(self, header):
        index = Index.create()
        tu = index.parse(header)
        self.tu = tu

        self.enum_map = {}
        for v in self.kinds('ENUM_DECL'):
            e = TAIEnum(v)
            self.enum_map[e.typename] = e

        v = self.get_name('_tai_attribute_value_t')
        self.attr_value_map = {}
        for f in v.get_children():
            # bool needs special handling since its type.spelling appears as 'int'
            key = 'bool' if f.displayname == 'booldata' else f.type.spelling
            self.attr_value_map[key] = f.displayname

        self.objects = [TAIObject(name, self) for name in ['module', 'host_interface', 'network_interface']]

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
            # ignore forward declaration
            # https://joshpeterson.github.io/identifying-a-forward-declaration-with-libclang
            d = node.get_definition()
            c = clang.cindex.conf.lib.clang_getNullCursor()
            if d != c and d == node:
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
{%- if defaultvalue %}
const tai_attribute_value_t tai_metadata_{{ typename }}_default_value = { .{{ value_field }} = {{ defaultvalue }} };
{%- endif %}
const tai_attr_metadata_t tai_metadata_attr_{{ typename }} = {
    .objecttype          = {{ object }},
    .attrid              = {{ typename }},
    .attridname          = "{{ typename }}",
    .attridshortname     = "{{ shorttypename }}",
    .attrvaluetype       = {{ attr_type }},
{%- if attrlist_value_type %}
    .attrlistvaluetype   = {{ attrlist_value_type }},
{%- endif %}
{%- if attr_flags %}
    .flags               = {{ attr_flags }},
{%- else %}
    .flags               = 0,
{%- endif %}
    .isenum              = {{ is_enum }},
{%- if enum_meta_data %}
    .enummetadata        = &{{ enum_meta_data }},
{%- else %}
    .enummetadata        = NULL,
{%- endif %}
    .ismandatoryoncreate = {{ ismandatoryoncreate }},
    .iscreateonly        = {{ iscreateonly }},
    .iscreateandset      = {{ iscreateandset }},
    .isreadonly          = {{ isreadonly }},
    .iskey               = {{ iskey }},
    .defaultvaluetype    = {{ defaultvaluetype }},
{%- if defaultvalue %}
    .defaultvalue        = &tai_metadata_{{ typename }}_default_value,
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
        attrlist_value_type = None
        if attr.attrlist_value_type:
            attrlist_value_type = 'TAI_ATTR_VALUE_TYPE_{}'.format(attr.attrlist_value_type.upper())
        is_enum = 'false'
        enum_meta_data = None
        if attr.enum_type:
            is_enum = 'true'
            enum_meta_data = 'tai_metadata_enum_{}'.format(attr.enum_type)
        attr_flags = None
        if attr.flags:
            attr_flags = '|'.join('TAI_ATTR_FLAGS_{}'.format(e.name) for e in list(attr.flags))

        default_value = None
        if attr.default_type == TAIDefaultValueType.CONST:
            default_value = attr.default

        self.data = {'object': obj,
                     'typename': typename,
                     'shorttypename': shorttypename,
                     'attr_type': attr_type,
                     'attrlist_value_type': attrlist_value_type,
                     'attr_flags': attr_flags,
                     'value_field': attr.value_field,
                     'is_enum': is_enum,
                     'enum_meta_data': enum_meta_data,
                     'ismandatoryoncreate': 'true' if TAIAttributeFlag.MANDATORY_ON_CREATE in attr.flags else 'false',
                     'iscreateonly': 'true' if TAIAttributeFlag.CREATE_ONLY in attr.flags else 'false',
                     'iscreateandset': 'true' if TAIAttributeFlag.CREATE_AND_SET in attr.flags else 'false',
                     'isreadonly': 'true' if TAIAttributeFlag.READ_ONLY in attr.flags else 'false',
                     'iskey': 'true' if TAIAttributeFlag.KEY in attr.flags else 'false',
                     'defaultvaluetype': 'TAI_DEFAULT_VALUE_TYPE_{}'.format(attr.default_type.name),
                     'defaultvalue': default_value,
                     }


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

    def __init__(self, header):
        generators = []
        all_attrs = []
        for obj in h.objects:
            attrs = obj.get_attributes()
            enums = obj.get_enums()

            for e in enums:
                g = EnumMetadataGenerator(e)
                generators.append(g)

            for a in attrs:
                g = AttrMetadataGenerator(a)
                generators.append(g)

            e = header.get_enum('tai_{}_attr_t'.format(obj.name))
            g = EnumMetadataGenerator(e)
            generators.append(g)

            g = ObjectMetadataGenerator(obj)
            generators.append(g)

            all_attrs += [a.name for a in attrs]

        self.data = {'impls': [g.implementation() for g in generators],
                     'headers': [x for x in [g.header() for g in generators] if len(x) > 0],
                     'object_names': [o.name for o in h.objects],
                     'attrs': sorted(all_attrs)}


if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option('--clang-lib', default='/usr/lib/llvm-6.0/lib/libclang.so.1')
    (options, args) = parser.parse_args()

    Config.set_library_file(options.clang_lib)

    h = TAIHeader(args[0])

    g = TAIMetadataGenerator(h)
    with open('taimetadata.h', 'w') as f:
        f.write(g.header())

    with open('taimetadata.c', 'w') as f:
        f.write(g.implementation())
