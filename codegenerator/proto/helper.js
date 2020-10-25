module.exports = {

    isTypeInData : function(data, typeName)
    {
        for (var i = 0; i < data.enums.length; i++)
        {
            if (data.enums[i].typeName == typeName)
            {
                return true;
            }
        }
        for (var i = 0; i < data.structs.length; i++)
        {
            if (data.structs[i].typeName == typeName)
            {
                return true;
            }
        }
        return false;
    },

    typeNameWithNamespace : function(data, typeName, delimiter)
    {
        var type = typeName;
        if (typeName.split('.').length > 1 || data.namespace == null || data.namespace.length == 0 || !this.isTypeInData(data, typeName))
        {
            type = 'fmq.' + typeName;
        }
        else
        {
            type = 'fmq.' + data.namespace + '.' + typeName
        }
        return type.replace('.', delimiter)
    },

    getPlainTypeName : function(typeName)
    {
        list = typeName.split('.')
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
     
    typeId2type : function(data, typeId, typeName, flags) { 

        switch (typeId)
        {
            case 'TYPE_BOOL':           return 'bool'
            case 'TYPE_INT32':          return (this.isVarint(flags)) ? 'int32'  : (this.isZigZag(flags)) ? 'sint32' : 'sfixed32';
            case 'TYPE_UINT32':         return (this.isVarint(flags)) ? 'uint32' : 'fixed32';
            case 'TYPE_INT64':          return (this.isVarint(flags)) ? 'int64'  : (this.isZigZag(flags)) ? 'sint64' : 'sfixed64';
            case 'TYPE_UINT64':         return (this.isVarint(flags)) ? 'uint64' : 'fixed64';
            case 'TYPE_FLOAT':          return 'float'
            case 'TYPE_DOUBLE':         return 'double'
            case 'TYPE_STRING':         return 'string'
            case 'TYPE_BYTES':          return 'bytes'
            case 'TYPE_STRUCT':         return this.typeNameWithNamespace(data, typeName, '.')
            case 'TYPE_ENUM':           return this.typeNameWithNamespace(data, typeName, '.')
            case 'TYPE_ARRAY_BOOL':     return 'repeated bool'
            case 'TYPE_ARRAY_INT32':    return 'repeated ' + ((this.isVarint(flags)) ? 'int32'  : (this.isZigZag(flags)) ? 'sint32' : 'sfixed32')
            case 'TYPE_ARRAY_UINT32':   return 'repeated ' + ((this.isVarint(flags)) ? 'uint32' : 'fixed32')
            case 'TYPE_ARRAY_INT64':    return 'repeated ' + ((this.isVarint(flags)) ? 'int64'  : (this.isZigZag(flags)) ? 'sint64' : 'sfixed64')
            case 'TYPE_ARRAY_UINT64':   return 'repeated ' + ((this.isVarint(flags)) ? 'uint64' : 'fixed64')
            case 'TYPE_ARRAY_FLOAT':    return 'repeated float'
            case 'TYPE_ARRAY_DOUBLE':   return 'repeated double'
            case 'TYPE_ARRAY_STRING':   return 'repeated string'
            case 'TYPE_ARRAY_BYTES':    return 'repeated bytes'
            case 'TYPE_ARRAY_STRUCT':   return 'repeated '+this.typeNameWithNamespace(data, typeName, '.')
            case 'TYPE_ARRAY_ENUM':     return 'repeated '+this.typeNameWithNamespace(data, typeName, '.')
        }
    },

}
