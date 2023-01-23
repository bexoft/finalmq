using System;
using System.Collections.Generic;


namespace finalmq
{
    public interface IParserVisitor
    {
        void NotifyError(string str, string message);
        void StartStruct(MetaStruct stru);
        void Finished();

        void EnterStruct(MetaField field);
        void ExitStruct(MetaField field);
        void EnterStructNull(MetaField field);

        void EnterArrayStruct(MetaField field);
        void ExitArrayStruct(MetaField field);

        void EnterBool(MetaField field, bool value);
        void EnterInt32(MetaField field, int value);
        void EnterUInt32(MetaField field, uint value);
        void EnterInt64(MetaField field, long value);
        void EnterUInt64(MetaField field, ulong value);
        void EnterFloat(MetaField field, float value);
        void EnterDouble(MetaField field, double value);
        void EnterString(MetaField field, string value);
        void EnterString(MetaField field, byte[] buffer, int offset, int size);
        void EnterBytes(MetaField field, byte[] value, int offset, int size);
        void EnterEnum(MetaField field, int value);
        void EnterEnum(MetaField field, string value);

        void EnterArrayBool(MetaField field, bool[] value);
        void EnterArrayInt32(MetaField field, int[] value);
        void EnterArrayUInt32(MetaField field, uint[] value);
        void EnterArrayInt64(MetaField field, long[] value);
        void EnterArrayUInt64(MetaField field, ulong[] value);
        void EnterArrayFloat(MetaField field, float[] value);
        void EnterArrayDouble(MetaField field, double[] value);
        void EnterArrayString(MetaField field, IList<string> value);
        void EnterArrayBytes(MetaField field, IList<byte[]> value);
        void EnterArrayEnum(MetaField field, int[] value);
        void EnterArrayEnum(MetaField field, IList<string> value);
    }

}