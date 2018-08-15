using System.Collections;
using System.Collections.Generic;

namespace Noesis.Javascript.Tests.Proxy
{
    public class JavaScriptDictionary<K, V> : IDictionary<K, V>
    {

        public JavaScriptDictionary(IDictionary<K, V> dict)
        {
            realDict = dict;
        }

        private IDictionary<K, V> realDict;

        public V this[K key]
        {
            get
            {
                return realDict[key];
            }
            set
            {
                realDict[key] = value;
            }
        }

        public ICollection<K> Keys
        {
            get
            {
                return realDict.Keys;
            }
        }

        public ICollection<V> Values
        {
            get
            {
                return realDict.Values;
            }
        }

        public int Count
        {
            get
            {
                return realDict.Count;
            }
        }

        public bool IsReadOnly
        {
            get
            {
                return realDict.IsReadOnly;
            }
        }

        public void Add(K key, V value)
        {
            realDict.Add(key, value);
        }

        public void Add(KeyValuePair<K, V> item)
        {
            realDict.Add(item);
        }

        public void Clear()
        {
            realDict.Clear();
        }

        public bool Contains(KeyValuePair<K, V> item)
        {
            return realDict.Contains(item);
        }

        public bool ContainsKey(K key)
        {
            return realDict.ContainsKey(key);
        }

        public void CopyTo(KeyValuePair<K, V>[] array, int arrayIndex)
        {
            realDict.CopyTo(array, arrayIndex);
        }

        public IEnumerator<KeyValuePair<K, V>> GetEnumerator()
        {
            return realDict.GetEnumerator();
        }

        public bool Remove(K key)
        {
            return realDict.Remove(key);
        }

        public bool Remove(KeyValuePair<K, V> item)
        {
            return realDict.Remove(item);
        }

        public bool TryGetValue(K key, out V value)
        {
            return realDict.TryGetValue(key, out value);
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return realDict.GetEnumerator();
        }
    }
}
