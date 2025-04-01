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
        var type = type;
        if (type.split('.').length > 1 || data.namespace == null || data.namespace.length == 0 || !this.isTypeInData(data, type))
        {
            type = 'fmq.' + type;
        }
        else
        {
            type = 'fmq.' + data.namespace + '.' + type
        }
        return type.replace('.', delimiter)
    },

    getPlainType : function(type)
    {
        list = type.split('.')
        return list[list.length - 1]
    },

    isVarint : function(flags)
    {
        for (var i = 0; i < flags.length; i++)
        {
            if (flags[i].trim() == 'METAFLAG_PROTO_VARINT')
            {
                return true;
            }
        }
        return false;
    },
     
    isZigZag : function(flags)
    {
        for (var i = 0; i < flags.length; i++)
        {
            if (flags[i].trim() == 'METAFLAG_PROTO_ZIGZAG')
            {
                return true;
            }
        }
        return false;
    },
     
    tid2type : function(data, tid, type, flags) { 

        switch (tid)
        {
            case 'TYPE_BOOL':           return 'bool'
            case 'TYPE_INT8':           return (this.isVarint(flags)) ? 'int32' : (this.isZigZag(flags)) ? 'sint32' : 'sfixed32';
            case 'TYPE_UINT8':          return (this.isVarint(flags)) ? 'uint32' : 'fixed32';
            case 'TYPE_INT16':          return (this.isVarint(flags)) ? 'int32' : (this.isZigZag(flags)) ? 'sint32' : 'sfixed32';
            case 'TYPE_UINT16':         return (this.isVarint(flags)) ? 'uint32' : 'fixed32';
            case 'TYPE_INT32':          return (this.isVarint(flags)) ? 'int32' : (this.isZigZag(flags)) ? 'sint32' : 'sfixed32';
            case 'TYPE_UINT32':         return (this.isVarint(flags)) ? 'uint32' : 'fixed32';
            case 'TYPE_INT64':          return (this.isVarint(flags)) ? 'int64'  : (this.isZigZag(flags)) ? 'sint64' : 'sfixed64';
            case 'TYPE_UINT64':         return (this.isVarint(flags)) ? 'uint64' : 'fixed64';
            case 'TYPE_FLOAT':          return 'float'
            case 'TYPE_DOUBLE':         return 'double'
            case 'TYPE_STRING':         return 'string'
            case 'TYPE_BYTES':          return 'bytes'
            case 'TYPE_STRUCT':         return this.typeWithNamespace(data, type, '.')
            case 'TYPE_ENUM':           return this.typeWithNamespace(data, type, '.')
            case 'TYPE_VARIANT':        return 'finalmq.variant.VarValue'
            case 'TYPE_JSON':           return 'string'
            case 'TYPE_ARRAY_BOOL':     return 'repeated bool'
            case 'TYPE_ARRAY_INT8':     return 'repeated ' + ((this.isVarint(flags)) ? 'int32' : (this.isZigZag(flags)) ? 'sint32' : 'sfixed32')
            case 'TYPE_ARRAY_INT16':    return 'repeated ' + ((this.isVarint(flags)) ? 'int32' : (this.isZigZag(flags)) ? 'sint32' : 'sfixed32')
            case 'TYPE_ARRAY_UINT16':   return 'repeated ' + ((this.isVarint(flags)) ? 'uint32' : 'fixed32')
            case 'TYPE_ARRAY_INT32':    return 'repeated ' + ((this.isVarint(flags)) ? 'int32' : (this.isZigZag(flags)) ? 'sint32' : 'sfixed32')
            case 'TYPE_ARRAY_UINT32':   return 'repeated ' + ((this.isVarint(flags)) ? 'uint32' : 'fixed32')
            case 'TYPE_ARRAY_INT64':    return 'repeated ' + ((this.isVarint(flags)) ? 'int64'  : (this.isZigZag(flags)) ? 'sint64' : 'sfixed64')
            case 'TYPE_ARRAY_UINT64':   return 'repeated ' + ((this.isVarint(flags)) ? 'uint64' : 'fixed64')
            case 'TYPE_ARRAY_FLOAT':    return 'repeated float'
            case 'TYPE_ARRAY_DOUBLE':   return 'repeated double'
            case 'TYPE_ARRAY_STRING':   return 'repeated string'
            case 'TYPE_ARRAY_BYTES':    return 'repeated bytes'
            case 'TYPE_ARRAY_STRUCT':   return 'repeated '+this.typeWithNamespace(data, type, '.')
            case 'TYPE_ARRAY_ENUM':     return 'repeated '+this.typeWithNamespace(data, type, '.')
        }
    },

	isVariantNeeded : function(structs) {
		for (var i = 0; i < structs.length; i++)
		{
			var stru = structs[i];
			for (var n = 0; n < stru.fields.length; n++) 
			{ 
				field = stru.fields[n];
				if (field.tid == 'TYPE_VARIANT')
				{
					return true;
				}
			}
		}
		return false;
	}
}
