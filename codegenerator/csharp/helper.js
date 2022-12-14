module.exports = {

    convertTypeId: function (data)
    {
        structs = data.structs;
        for (var i = 0; i < structs.length; i++)
        {
            var stru = structs[i];
            for (var n = 0; n < stru.fields.length; n++)
            {
                field = stru.fields[n];
                switch (field.tid)
                {
                    case 'bool': field.tid = 'TYPE_BOOL'; break;
                    case 'int32': field.tid = 'TYPE_INT32'; break;
                    case 'uint32': field.tid = 'TYPE_UINT32'; break;
                    case 'int64': field.tid = 'TYPE_INT64'; break;
                    case 'uint64': field.tid = 'TYPE_UINT64'; break;
                    case 'float': field.tid = 'TYPE_FLOAT'; break;
                    case 'double': field.tid = 'TYPE_DOUBLE'; break;
                    case 'string': field.tid = 'TYPE_STRING'; break;
                    case 'bytes': field.tid = 'TYPE_BYTES'; break;
                    case 'struct': field.tid = 'TYPE_STRUCT'; break;
                    case 'enum': field.tid = 'TYPE_ENUM'; break;
                    case 'variant': field.tid = 'TYPE_VARIANT'; break;
                    case 'bool[]': field.tid = 'TYPE_ARRAY_BOOL'; break;
                    case 'int32[]': field.tid = 'TYPE_ARRAY_INT32'; break;
                    case 'uint32[]': field.tid = 'TYPE_ARRAY_UINT32'; break;
                    case 'int64[]': field.tid = 'TYPE_ARRAY_INT64'; break;
                    case 'uint64[]': field.tid = 'TYPE_ARRAY_UINT64'; break;
                    case 'float[]': field.tid = 'TYPE_ARRAY_FLOAT'; break;
                    case 'double[]': field.tid = 'TYPE_ARRAY_DOUBLE'; break;
                    case 'string[]': field.tid = 'TYPE_ARRAY_STRING'; break;
                    case 'bytes[]': field.tid = 'TYPE_ARRAY_BYTES'; break;
                    case 'struct[]': field.tid = 'TYPE_ARRAY_STRUCT'; break;
                    case 'enum[]': field.tid = 'TYPE_ARRAY_ENUM'; break;
                }
            }
        }
    },

    isTypeInData : function(data, type)
    {
        for (var i = 0; i < data.enums.length; i++)
        {
            if (data.enums[i].type == type)
            {
                return true;
            }
        }
        for (var i = 0; i < data.structs.length; i++)
        {
            if (data.structs[i].type == type)
            {
                return true;
            }
        }
        return false;
    },

    typeWithNamespace : function(data, type, delimiter)
    {
        if (type)
        { 
            var type = type;
            if (type.split('.').length > 1 || data.namespace == null || data.namespace.length == 0 || !this.isTypeInData(data, type))
            {
                type = type;
            }
            else
            {
                type = data.namespace + '.' + type
            }
            return type.split('.').join(delimiter)
        }
        else
        {
            return ""
        }
    },

    getPlainType : function(type)
    {
        list = type.split('.')
        return list[list.length - 1]
    },
     
    tid2type : function(data, tid, type) { 

        switch (tid)
        {
            case 'bool':
            case 'TYPE_BOOL': return 'bool'
            case 'int32':
            case 'TYPE_INT32': return 'int'
            case 'uint32':
            case 'TYPE_UINT32': return 'uint'
            case 'int64':
            case 'TYPE_INT64': return 'long'
            case 'uint64':
            case 'TYPE_UINT64': return 'ulong'
            case 'float':
            case 'TYPE_FLOAT': return 'float'
            case 'double':
            case 'TYPE_DOUBLE': return 'double'
            case 'string':
            case 'TYPE_STRING': return 'string'
            case 'bytes':
            case 'TYPE_BYTES': return 'byte[]'
            case 'struct':
            case 'TYPE_STRUCT': return this.typeWithNamespace(data, type, '.')
            case 'enum':
            case 'TYPE_ENUM': return this.typeWithNamespace(data, type, '.')
            case 'variant':
            case 'TYPE_VARIANT': return 'finalmq.Variant'
            case 'bool[]':
            case 'TYPE_ARRAY_BOOL': return 'bool[]'
            case 'int32[]':
            case 'TYPE_ARRAY_INT32': return 'int[]'
            case 'uint32[]':
            case 'TYPE_ARRAY_UINT32': return 'uint[]'
            case 'int64[]':
            case 'TYPE_ARRAY_INT64': return 'long[]'
            case 'uint64[]':
            case 'TYPE_ARRAY_UINT64': return 'ulong[]'
            case 'float[]':
            case 'TYPE_ARRAY_FLOAT': return 'float[]'
            case 'double[]':
            case 'TYPE_ARRAY_DOUBLE': return 'double[]'
            case 'string[]':
            case 'TYPE_ARRAY_STRING': return 'IList<string>'
            case 'bytes[]':
            case 'TYPE_ARRAY_BYTES': return 'IList<byte[]>'
            case 'struct[]':
            case 'TYPE_ARRAY_STRUCT': return 'IList<' + this.typeWithNamespace(data, type, '.') + '>'
            case 'enum[]':
            case 'TYPE_ARRAY_ENUM': return 'IList<' + this.typeWithNamespace(data, type, '.') + '>'
        }
    },

    tid2default : function(data, tid, type) { 

        switch (tid)
        {
            case 'bool':
            case 'TYPE_BOOL': return 'false'
            case 'int32':
            case 'TYPE_INT32': return '0'
            case 'uint32':
            case 'TYPE_UINT32': return '0'
            case 'int64':
            case 'TYPE_INT64': return '0'
            case 'uint64':
            case 'TYPE_UINT64': return '0'
            case 'float':
            case 'TYPE_FLOAT': return '0.0f'
            case 'double':
            case 'TYPE_DOUBLE': return '0.0'
            case 'string':
            case 'TYPE_STRING': return '\"\"'
            case 'bytes':
            case 'TYPE_BYTES': return 'new byte[0]'
            case 'struct':
            case 'TYPE_STRUCT': return 'new ' + this.tid2type(data, tid, type) + '()'
            case 'enum':
            case 'TYPE_ENUM': return 'new ' + this.tid2type(data, tid, type) + '()'
            case 'variant':
            case 'TYPE_VARIANT': return 'new finalmq.Variant()'
            case 'bool[]':
            case 'TYPE_ARRAY_BOOL': return 'new bool[0]'
            case 'int32[]':
            case 'TYPE_ARRAY_INT32': return 'new int[0]'
            case 'uint32[]':
            case 'TYPE_ARRAY_UINT32': return 'new uint[0]'
            case 'int64[]':
            case 'TYPE_ARRAY_INT64': return 'new long[0]'
            case 'uint64[]':
            case 'TYPE_ARRAY_UINT64': return 'new ulong[0]'
            case 'float[]':
            case 'TYPE_ARRAY_FLOAT': return 'new float[0]'
            case 'double[]':
            case 'TYPE_ARRAY_DOUBLE': return 'new double[0]'
            case 'string[]':
            case 'TYPE_ARRAY_STRING': return 'new string[0]'
            case 'bytes[]':
            case 'TYPE_ARRAY_BYTES': return 'new List<byte[]>()'
            case 'struct[]':
            case 'TYPE_ARRAY_STRUCT': return 'new List<' + this.typeWithNamespace(data, type, '.') + '>()'
            case 'enum[]':
            case 'TYPE_ARRAY_ENUM': return 'new List<' + this.typeWithNamespace(data, type, '.') + '>()'
        }
    },
   
    convertFlags : function(flagArray)
    {
        var flags = 'finalmq::METAFLAG_NONE'
        if (flagArray)
        {
            for (var i = 0; i < flagArray.length; i++)
            {
				if (flagArray[i].startsWith('METAFLAG_'))
				{
					flags += ' | finalmq::' + flagArray[i]
				}
            }
        }
        return flags
    },

    getOffset : function(tid)
    {
        if (tid == 'TYPE_STRUCT')
        {
            return 'OFFSET_STRUCTBASE_TO_STRUCTBASE';
        }
        return 'OFFSET_STRUCTBASE_TO_PARAM';
    },

    getDefaultEnum : function(entries)
    {
        for (var i = 0; i < entries.length; i++)
        {
            if (entries[i].id == 0)
            {
                return entries[i]
            }
        }
        return entries[0]
    }
     
}
