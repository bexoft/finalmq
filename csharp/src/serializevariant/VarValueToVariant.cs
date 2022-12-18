using System;
using System.Collections.Generic;
using System.Diagnostics;


namespace finalmq
{
    using VariantStruct = List<NameValue>;
    using VariantList = List<Variant>;

    class VarValueToVariant
    {
        public delegate void FuncExit();
        public VarValueToVariant(Variant variant)
        {
            m_variant = variant;
            m_varValue = new variant.VarValue();
            m_serializerStruct = new SerializerStruct(m_varValue);
        }

        public IParserVisitor GetVisitor()
        {
            return m_serializerStruct;
        }

        public void SetExitNotification(FuncExit? funcExit)
        {
            m_serializerStruct.SetExitNotification(funcExit);
        }
        public void Convert()
        {
            ProcessVarValue(m_varValue, m_variant);
        }

        void ProcessVarValue(variant.VarValue varValue, Variant var)
        {
            switch (varValue.type)
            {
                case variant.VarTypeId.T_NONE:
                    break;
                case variant.VarTypeId.T_BOOL:
                    var.SetData(varValue.valbool);
                    break;
                case variant.VarTypeId.T_INT32:
                    var.SetData(varValue.valint32);
                    break;
                case variant.VarTypeId.T_UINT32:
                    var.SetData(varValue.valuint32);
                    break;
                case variant.VarTypeId.T_INT64:
                    var.SetData(varValue.valint64);
                    break;
                case variant.VarTypeId.T_UINT64:
                    var.SetData(varValue.valuint64);
                    break;
                case variant.VarTypeId.T_FLOAT:
                    var.SetData(varValue.valfloat);
                    break;
                case variant.VarTypeId.T_DOUBLE:
                    var.SetData(varValue.valdouble);
                    break;
                case variant.VarTypeId.T_STRING:
                    var.SetData(varValue.valstring);
                    break;
                case variant.VarTypeId.T_BYTES:
                    var.SetData(varValue.valbytes);
                    break;
                case variant.VarTypeId.T_STRUCT:
                    VariantStruct variantStruct = new VariantStruct();
                    Debug.Assert(variantStruct != null);
                    foreach (var varValueElement in varValue.vallist)
                    {
                        Variant v = new Variant();
                        ProcessVarValue(varValueElement, v);
                        variantStruct.Add(new NameValue(varValueElement.name, v));
                    }
                    var.SetData(variantStruct);
                    break;

                case variant.VarTypeId.T_ARRAY_BOOL:
                    var.SetData(varValue.valarrbool);
                    break;
                case variant.VarTypeId.T_ARRAY_INT32:
                    var.SetData(varValue.valarrint32);
                    break;
                case variant.VarTypeId.T_ARRAY_UINT32:
                    var.SetData(varValue.valarruint32);
                    break;
                case variant.VarTypeId.T_ARRAY_INT64:
                    var.SetData(varValue.valarrint64);
                    break;
                case variant.VarTypeId.T_ARRAY_UINT64:
                    var.SetData(varValue.valarruint64);
                    break;
                case variant.VarTypeId.T_ARRAY_FLOAT:
                    var.SetData(varValue.valarrfloat);
                    break;
                case variant.VarTypeId.T_ARRAY_DOUBLE:
                    var.SetData(varValue.valarrdouble);
                    break;
                case variant.VarTypeId.T_ARRAY_STRING:
                    var.SetData(varValue.valarrstring);
                    break;
                case variant.VarTypeId.T_ARRAY_BYTES:
                    var.SetData(varValue.valarrbytes);
                    break;
                case variant.VarTypeId.T_LIST:
                    VariantList variantList = new VariantList();
                    foreach (var varValueElement in varValue.vallist)
                    {
                        Variant v = new Variant();
                        ProcessVarValue(varValueElement, v);
                        variantList.Add(v);
                    }
                    var.SetData(variantList);
                    break;
            }
        }

        readonly Variant m_variant;
        readonly SerializerStruct m_serializerStruct;
        readonly variant.VarValue m_varValue;
    };

}
