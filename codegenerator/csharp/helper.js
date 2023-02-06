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
     
    isNullable: function (field) {
        var flagArray = field.flags;
        if (flagArray) {
            for (var i = 0; i < flagArray.length; i++) {
                if (flagArray[i] == 'METAFLAG_NULLABLE') {
                    return true;
                }
            }
        }
        return false;
    },

    tid2type : function(data, field) {

        var tid = field.tid;
        var type = field.type;
        var nullable = this.isNullable(field);
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
            case 'TYPE_STRUCT':
                if (!nullable) {
                    return this.typeWithNamespace(data, type, '.')
                }
                else {
                    return this.typeWithNamespace(data, type, '.') + '?'
                }
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

	// return
	// 0: compare with ==
	// 1: compare with Equals
	// 2: compare with SequenceEqual
	// 3: compare list of arrays with SequenceEqual (only IList<byte[]>
	// 4: compare as nullable struct
    tid2CompareType : function(field) {

        var tid = field.tid;
        var nullable = this.isNullable(field);

        switch (tid)
        {
            case 'bool':
            case 'TYPE_BOOL': return 0;
            case 'int32':
            case 'TYPE_INT32': return 0;
            case 'uint32':
            case 'TYPE_UINT32': return 0;
            case 'int64':
            case 'TYPE_INT64': return 0;
            case 'uint64':
            case 'TYPE_UINT64': return 0;
            case 'float':
            case 'TYPE_FLOAT': return 0;
            case 'double':
            case 'TYPE_DOUBLE': return 0;
            case 'string':
            case 'TYPE_STRING': return 0;
            case 'bytes':
            case 'TYPE_BYTES': return 2;
            case 'struct':
            case 'TYPE_STRUCT': 
                if (!nullable) {
                    return 1;
                }
                else {
                    return 4;
                }
            case 'enum':
            case 'TYPE_ENUM': return 0;
            case 'variant':
            case 'TYPE_VARIANT': return 1;
            case 'bool[]':
            case 'TYPE_ARRAY_BOOL': return 2;
            case 'int32[]':
            case 'TYPE_ARRAY_INT32': return 2;
            case 'uint32[]':
            case 'TYPE_ARRAY_UINT32': return 2;
            case 'int64[]':
            case 'TYPE_ARRAY_INT64': return 2;
            case 'uint64[]':
            case 'TYPE_ARRAY_UINT64': return 2;
            case 'float[]':
            case 'TYPE_ARRAY_FLOAT': return 2;
            case 'double[]':
            case 'TYPE_ARRAY_DOUBLE': return 2;
            case 'string[]':
            case 'TYPE_ARRAY_STRING': return 2;
            case 'bytes[]':
            case 'TYPE_ARRAY_BYTES': return 3;
            case 'struct[]':
            case 'TYPE_ARRAY_STRUCT': return 2;
            case 'enum[]':
            case 'TYPE_ARRAY_ENUM': return 2;
        }
    },

    tid2default : function(data, field) { 

        var tid = field.tid;
        var type = field.type;
        var nullable = this.isNullable(field);

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
            case 'TYPE_BYTES': return 'Array.Empty<byte>()'
            case 'struct':
            case 'TYPE_STRUCT': 
                if (!nullable) {
                    return 'new ' + this.tid2type(data, field) + '()'
                }
                else {
                    return 'null'
                }
            case 'enum':
            case 'TYPE_ENUM': return 'new ' + this.tid2type(data, field) + '()'
            case 'variant':
            case 'TYPE_VARIANT': return 'new finalmq.Variant()'
            case 'bool[]':
            case 'TYPE_ARRAY_BOOL': return 'Array.Empty<bool>()'
            case 'int32[]':
            case 'TYPE_ARRAY_INT32': return 'Array.Empty<int>()'
            case 'uint32[]':
            case 'TYPE_ARRAY_UINT32': return 'Array.Empty<uint>()'
            case 'int64[]':
            case 'TYPE_ARRAY_INT64': return 'Array.Empty<long>()'
            case 'uint64[]':
            case 'TYPE_ARRAY_UINT64': return 'Array.Empty<ulong>()'
            case 'float[]':
            case 'TYPE_ARRAY_FLOAT': return 'Array.Empty<float>()'
            case 'double[]':
            case 'TYPE_ARRAY_DOUBLE': return 'Array.Empty<double>()'
            case 'string[]':
            case 'TYPE_ARRAY_STRING': return 'new List<string>()'
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
