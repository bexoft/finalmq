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
                    case 'int8': field.tid = 'TYPE_INT8'; break;
                    case 'uint8': field.tid = 'TYPE_UINT8'; break;
                    case 'int16': field.tid = 'TYPE_INT16'; break;
                    case 'uint16': field.tid = 'TYPE_UINT16'; break;
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
                    case 'json': field.tid = 'TYPE_JSON'; break;
                    case 'bool[]': field.tid = 'TYPE_ARRAY_BOOL'; break;
                    case 'int8[]': field.tid = 'TYPE_ARRAY_INT8'; break;
                    case 'int16[]': field.tid = 'TYPE_ARRAY_INT16'; break;
                    case 'uint16[]': field.tid = 'TYPE_ARRAY_UINT16'; break;
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
            var typeWithNamespace = type;
            if (type.split('.').length > 1 || data.namespace == null || data.namespace.length == 0 || !this.isTypeInData(data, type))
            {
                typeWithNamespace = type;
            }
            else
            {
                typeWithNamespace = data.namespace + '.' + type
            }
            return typeWithNamespace.split('.').join(delimiter)
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
	
	isNullable : function(field)
	{
		var flagArray = field.flags;
		if (flagArray)
		{
			for (var i = 0; i < flagArray.length; i++)
			{
				if (flagArray[i] == 'METAFLAG_NULLABLE')
				{
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
            case 'int8':
            case 'TYPE_INT8': return 'std::int8_t'
            case 'uint8':
            case 'TYPE_UINT8': return 'std::uint8_t'
            case 'int16':
            case 'TYPE_INT16': return 'std::int16_t'
            case 'uint15':
            case 'TYPE_UINT16': return 'std::uint16_t'
            case 'int32':
            case 'TYPE_INT32': return 'std::int32_t'
            case 'uint32':
            case 'TYPE_UINT32': return 'std::uint32_t'
            case 'int64':
            case 'TYPE_INT64': return 'std::int64_t'
            case 'uint64':
            case 'TYPE_UINT64': return 'std::uint64_t'
            case 'float':
            case 'TYPE_FLOAT': return 'float'
            case 'double':
            case 'TYPE_DOUBLE': return 'double'
            case 'string':
            case 'TYPE_STRING': return 'std::string'
            case 'bytes':
            case 'TYPE_BYTES': return 'finalmq::Bytes'
            case 'struct':
            case 'TYPE_STRUCT': 
				if (!nullable)
				{
					return this.typeWithNamespace(data, type, '::')
				}
				else
				{
					return 'std::shared_ptr<' + this.typeWithNamespace(data, type, '::') + '>';
				}
            case 'enum':
            case 'TYPE_ENUM': return this.typeWithNamespace(data, type, '::')
            case 'variant':
            case 'TYPE_VARIANT': return 'finalmq::Variant'
            case 'json':
            case 'TYPE_JSON': return 'finalmq::Variant'
            case 'bool[]':
            case 'TYPE_ARRAY_BOOL': return 'std::vector<bool>'
            case 'int8[]':
            case 'TYPE_ARRAY_INT8': return 'std::vector<std::int8_t>'
            case 'int16[]':
            case 'TYPE_ARRAY_INT16': return 'std::vector<std::int16_t>'
            case 'uint16[]':
            case 'TYPE_ARRAY_UINT16': return 'std::vector<std::uint16_t>'
            case 'int32[]':
            case 'TYPE_ARRAY_INT32': return 'std::vector<std::int32_t>'
            case 'uint32[]':
            case 'TYPE_ARRAY_UINT32': return 'std::vector<std::uint32_t>'
            case 'int64[]':
            case 'TYPE_ARRAY_INT64': return 'std::vector<std::int64_t>'
            case 'uint64[]':
            case 'TYPE_ARRAY_UINT64': return 'std::vector<std::uint64_t>'
            case 'float[]':
            case 'TYPE_ARRAY_FLOAT': return 'std::vector<float>'
            case 'double[]':
            case 'TYPE_ARRAY_DOUBLE': return 'std::vector<double>'
            case 'string[]':
            case 'TYPE_ARRAY_STRING': return 'std::vector<std::string>'
            case 'bytes[]':
            case 'TYPE_ARRAY_BYTES': return 'std::vector<finalmq::Bytes>'
            case 'struct[]':
            case 'TYPE_ARRAY_STRUCT': return 'std::vector<' + this.typeWithNamespace(data, type, '::') + '>'
            case 'enum[]':
            case 'TYPE_ARRAY_ENUM': return 'std::vector<' + this.typeWithNamespace(data, type, '::') + '>'
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

    convertStructFlags : function(flagArray)
    {
        var flags = 'finalmq::METASTRUCTFLAG_NONE'
        if (flagArray)
        {
            for (var i = 0; i < flagArray.length; i++)
            {
				if (flagArray[i].startsWith('METASTRUCTFLAG_'))
				{
					flags += ' | finalmq::' + flagArray[i]
				}
            }
        }
        return flags
    },

    convertAttrs: function (attrArray) {
        var attrs = '{'
        if (attrArray) {
            for (var i = 0; i < attrArray.length; i++) {
                attrs += '"' + attrArray[i] + '"'
                if (i < attrArray.length - 1) {
                    attrs += ', '
                }
            }
        }
        attrs += '}'
        return attrs
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
    },

    avoidCppKeyWords: function (name)
    {
        if (name == 'namespace') {
            return name + '_';
        }
        return name;
    }

}
