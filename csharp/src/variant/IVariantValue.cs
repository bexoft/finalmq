using System;
using System.Collections.Generic;


namespace finalmq
{
    public interface IVariantVisitor
    {
        void EnterLeaf(Variant variant, int type, int index, int level, int size, string name);
        void EnterStruct(Variant variant, int type, int index, int level, int size, string name);
        void ExitStruct(Variant variant, int type, int index, int level, int size, string name);
        void EnterList(Variant variant, int type, int index, int level, int size, string name);
        void ExitList(Variant variant, int type, int index, int level, int size, string name);
    };



    public interface IVariantValue
    {
        int VarType { get; }
        dynamic Data { get; set; }
        Variant? GetVariant(string name);
        IVariantValue Clone();
        bool Equals(IVariantValue rhs);
        bool Add(string name, Variant variant);
        bool Add(Variant variant);
        int Length { get; }
        void Accept(IVariantVisitor visitor, Variant variant, int index, int level, int size, string name);
    };
}
