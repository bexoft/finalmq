module.exports = {

    getPlainType : function(type)
    {
        list = type.split('.')
        return list[list.length - 1]
    },
     
    tid2type : function(data, field) {

        var tid = field.tid;
        var type = field.type;
        switch (tid)
        {
            case 'bool':
            case 'TYPE_BOOL': return 'bool'
            case 'int8':
            case 'TYPE_INT8': return 'int'
            case 'uint8':
            case 'TYPE_UINT8': return 'int'
            case 'int16':
            case 'TYPE_INT16': return 'int'
            case 'uint16':
            case 'TYPE_UINT16': return 'int'
            case 'int32':
            case 'TYPE_INT32': return 'int'
            case 'uint32':
            case 'TYPE_UINT32': return 'int'
            case 'int64':
            case 'TYPE_INT64': return 'int'
            case 'uint64':
            case 'TYPE_UINT64': return 'int'
            case 'float':
            case 'TYPE_FLOAT': return 'float'
            case 'double':
            case 'TYPE_DOUBLE': return 'float'
            case 'string':
            case 'TYPE_STRING': return "''"
            case 'bytes':
            case 'TYPE_BYTES': return 'bytearray()'
            case 'struct':
            case 'TYPE_STRUCT': return type
            case 'enum':
            case 'TYPE_ENUM': return type
            case 'variant':
            case 'TYPE_VARIANT': return '{}'
            case 'bool[]':
            case 'TYPE_ARRAY_BOOL': return '[]'
            case 'int8[]':
            case 'TYPE_ARRAY_INT8': return '[]'
            case 'int16[]':
            case 'TYPE_ARRAY_INT16': return '[]'
            case 'uint16[]':
            case 'TYPE_ARRAY_UINT16': return '[]'
            case 'int32[]':
            case 'TYPE_ARRAY_INT32': return '[]'
            case 'uint32[]':
            case 'TYPE_ARRAY_UINT32': return '[]'
            case 'int64[]':
            case 'TYPE_ARRAY_INT64': return '[]'
            case 'uint64[]':
            case 'TYPE_ARRAY_UINT64': return '[]'
            case 'float[]':
            case 'TYPE_ARRAY_FLOAT': return '[]'
            case 'double[]':
            case 'TYPE_ARRAY_DOUBLE': return '[]'
            case 'string[]':
            case 'TYPE_ARRAY_STRING': return '[]'
            case 'bytes[]':
            case 'TYPE_ARRAY_BYTES': return '[]'
            case 'struct[]':
            case 'TYPE_ARRAY_STRUCT': return '[]'
            case 'enum[]':
            case 'TYPE_ARRAY_ENUM': return '[]'
        }
    },

}
