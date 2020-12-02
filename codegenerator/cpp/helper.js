module.exports = {

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
            type = type;
        }
        else
        {
            type = data.namespace + '.' + type
        }
        return type.split('.').join(delimiter)
    },

    getPlainType : function(type)
    {
        list = type.split('.')
        return list[list.length - 1]
    },
     
    tid2type : function(data, tid, type) { 

        switch (tid)
        {
            case 'TYPE_BOOL':           return 'bool'
            case 'TYPE_INT32':          return 'std::int32_t'
            case 'TYPE_UINT32':         return 'std::uint32_t'
            case 'TYPE_INT64':          return 'std::int64_t'
            case 'TYPE_UINT64':         return 'std::uint64_t'
            case 'TYPE_FLOAT':          return 'float'
            case 'TYPE_DOUBLE':         return 'double'
            case 'TYPE_STRING':         return 'std::string'
            case 'TYPE_BYTES':          return 'finalmq::Bytes'
            case 'TYPE_STRUCT':         return this.typeWithNamespace(data, type, '::')
            case 'TYPE_ENUM':           return this.typeWithNamespace(data, type, '::')
            case 'TYPE_ARRAY_BOOL':     return 'std::vector<bool>'
            case 'TYPE_ARRAY_INT32':    return 'std::vector<std::int32_t>'
            case 'TYPE_ARRAY_UINT32':   return 'std::vector<std::uint32_t>'
            case 'TYPE_ARRAY_INT64':    return 'std::vector<std::int64_t>'
            case 'TYPE_ARRAY_UINT64':   return 'std::vector<std::uint64_t>'
            case 'TYPE_ARRAY_FLOAT':    return 'std::vector<float>'
            case 'TYPE_ARRAY_DOUBLE':   return 'std::vector<double>'
            case 'TYPE_ARRAY_STRING':   return 'std::vector<std::string>'
            case 'TYPE_ARRAY_BYTES':    return 'std::vector<finalmq::Bytes>'
            case 'TYPE_ARRAY_STRUCT':   return 'std::vector<'+this.typeWithNamespace(data, type, '::')+'>'
            case 'TYPE_ARRAY_ENUM':     return 'std::vector<'+this.typeWithNamespace(data, type, '::')+'>'
        }
    },

    tid2default : function(tid) { 

        switch (tid)
        {
            case 'TYPE_BOOL':           return 'false'
            case 'TYPE_INT32':          return '0'
            case 'TYPE_UINT32':         return '0'
            case 'TYPE_INT64':          return '0'
            case 'TYPE_UINT64':         return '0'
            case 'TYPE_FLOAT':          return '0.0'
            case 'TYPE_DOUBLE':         return '0.0'
            case 'TYPE_STRING':         return null
            case 'TYPE_BYTES':          return null
            case 'TYPE_STRUCT':         return null
            case 'TYPE_ENUM':           return null
            case 'TYPE_ARRAY_BOOL':     return null
            case 'TYPE_ARRAY_INT32':    return null
            case 'TYPE_ARRAY_UINT32':   return null
            case 'TYPE_ARRAY_INT64':    return null
            case 'TYPE_ARRAY_UINT64':   return null
            case 'TYPE_ARRAY_FLOAT':    return null
            case 'TYPE_ARRAY_DOUBLE':   return null
            case 'TYPE_ARRAY_STRING':   return null
            case 'TYPE_ARRAY_BYTES':    return null
            case 'TYPE_ARRAY_STRUCT':   return null
            case 'TYPE_ARRAY_ENUM':     return null
        }
    },
   
    convertFlags : function(flagArray)
    {
        var flags = 'finalmq::METAFLAG_NONE'
        for (var i = 0; i < flagArray.length; i++)
        {
            flags += ' | finalmq::' + flagArray[i]
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
