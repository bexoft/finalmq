
#include "StructsMetaData.h"



class MetaDataExchange
{
public:
    static void importMetaData(const SerializeMetaData& metadata);
    static void exportMetaData(SerializeMetaData& metadata);

    static void importMetaDataJson(const char* json);
    static void exportMetaDataJson(std::string& json);
private:
};

