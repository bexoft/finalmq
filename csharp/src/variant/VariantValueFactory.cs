﻿using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;


namespace finalmq
{
    public interface IVariantValueFactory
    {
        delegate IVariantValue FuncCreateVariantValue(dynamic data);
        IVariantValue? CreateVariantValue<T>(T data);
        void Register<T>(FuncCreateVariantValue func);
        int GetVarType(Type type);
    };

    public class VariantValueFactoryImpl : IVariantValueFactory
    {
        public VariantValueFactoryImpl()
        {
        }

        private class Entry
        {
            public Entry(int vartype, IVariantValueFactory.FuncCreateVariantValue func)
            {
                m_vartype = vartype;
                m_func = func;
            }

            public int VarType
            {
                get { return m_vartype; }
            }

            public IVariantValueFactory.FuncCreateVariantValue Func
            {
                get { return m_func; }
            }

            readonly int m_vartype;
            readonly IVariantValueFactory.FuncCreateVariantValue m_func;
        }

        // IVariantValueFactory
        public IVariantValue? CreateVariantValue<T>(T data)
        {
            Debug.Assert(data != null);
            Type? type = typeof(T);
            Entry? entry = null;
            m_factoryMethods.TryGetValue(type, out entry);
            if (entry != null)
            {
                Debug.Assert(entry.Func != null);
                return entry.Func(data);
            }
            else if (type.BaseType == null || type.BaseType.FullName != "System.Array")
            {
                Type[] interfaces = type.GetInterfaces();
                foreach(var interf in interfaces)
                {
                    m_factoryMethods.TryGetValue(interf, out entry);
                    if (entry != null)
                    {
                        Debug.Assert(entry.Func != null);
                        return entry.Func(data);
                    }
                }
            }
            while (type != null)
            {
                type = type.BaseType;
                if (type != null)
                {
                    m_factoryMethods.TryGetValue(type, out entry);
                    if (entry != null)
                    {
                        Debug.Assert(entry.Func != null);
                        return entry.Func(data);
                    }
                }
            }
            return null;
        }

        public void Register<T>(IVariantValueFactory.FuncCreateVariantValue func)
        {
            Debug.Assert(func != null);
#pragma warning disable CS8604 // Mögliches Nullverweisargument.
            int vartype = func(default(T)).VarType;
#pragma warning restore CS8604 // Mögliches Nullverweisargument.
            m_factoryMethods.Add(typeof(T), new Entry(vartype, func));
        }
        
        public int GetVarType(Type type)
        {
            Entry? entry = null;
            m_factoryMethods.TryGetValue(type, out entry);
            if (entry != null)
            {
                return entry.VarType;
            }
            return 0;
        }


        IDictionary<Type, Entry> m_factoryMethods = new Dictionary<Type, Entry>();
    };


    public class VariantValueFactory
    {
        public static IVariantValueFactory Instance
        {
            get => m_instance;
            set
            {
                m_instance = value;
            }
        }
        private VariantValueFactory()
        {
        }

        private static IVariantValueFactory m_instance = new VariantValueFactoryImpl();
    };
}
