
#include "metadataserialize/metadata.fmq.h"



class MetaDataExchange
{
public:
    static void importMetaData(const finalmq::SerializeMetaData& metadata);
    static void exportMetaData(finalmq::SerializeMetaData& metadata);

    static void importMetaDataJson(const char* json);
    static void exportMetaDataJson(std::string& json);

    static void importMetaDataProto(const char* proto, int size);
    static void exportMetaDataProto(std::string& proto);

private:
};

