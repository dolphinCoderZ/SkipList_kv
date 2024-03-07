#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <fstream>
#include <mutex>

#define STORE_FILE "store/dump_file"
using namespace std;

// �ٽ�����
mutex mtx, mtx1;
// �ָ���
string delimter = ":";

// ����ڵ�
template <typename K, typename V>
class Node
{
public:
    Node(){};
    Node(K key, V value, int level);
    ~Node();

    K getKey() const;
    V getValue() const;

    void setValue(V value);

    // ���ڲ�ͬ�㣬����ָ�����ڵ㣬ָ�����鱣��
    Node<K, V> **forward;
    int nodeLevel;

private:
    K key;
    V value;
};

template <typename K, typename V>
Node<K, V>::Node(const K key, const V value, int level)
{
    this->key = key;
    this->value = value;
    this->nodeLevel = level;

    // ��������ָ����һ�ڵ��ָ��
    this->forward = new Node<K, V> *[level + 1];
    memset(this->forward, 0, sizeof(Node<K, V> *) * (level + 1));
}

template <typename K, typename V>
Node<K, V>::~Node()
{
    delete[] this->forward;
}

template <typename K, typename V>
K Node<K, V>::getKey() const
{
    return this->key;
}

template <typename K, typename V>
V Node<K, V>::getValue() const
{
    return this->value;
}

template <typename K, typename V>
void Node<K, V>::setValue(V value)
{
    this->value = value;
}

template <typename K, typename V>
class skipList
{
public:
    skipList(int level);
    ~skipList();
    // �����������
    int getRandomLevel();

    Node<K, V> *createNode(K key, V value, int level);
    // ��ɾ�Ĳ�
    int insertElem(K key, V value);
    int updateElem(K key, V value, bool needCreate = false);
    bool searchElem(K key);
    void deleteElem(K key);
    // �������̺����ݼ���
    void dumpFile();
    void loadFile();

    void displayList();
    void clear();
    int size();

private:
    void getKVFromString(string &str, string *key, string *value);
    bool isValidString(string &str);

private:
    // �����������
    int maxLevel;
    // ��ǰ�������߲�
    int curLevel;

    Node<K, V> *head;
    ofstream fileWriter;
    ifstream fileReader;
    // �����Ԫ�ظ���
    int elemCount;
};

template <typename K, typename V>
Node<K, V> *skipList<K, V>::createNode(const K key, const V value, int level)
{
    Node<K, V> *node = new Node<K, V>(key, value, level);
    return node;
}

template <typename K, typename V>
// ����ֵ��-1��ʾԪ�ش��ڣ�0��ʾ����ɹ�
int skipList<K, V>::insertElem(K key, V value)
{
    mtx.lock();
    // ӵ�����ָ��Ľڵ㣬��ָ���������Ŀ�����maxLevel+1
    Node<K, V> *update[this->maxLevel + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (this->maxLevel + 1));
    // ������
    // ��ÿһ��ĵ�һ���ڵ㿪ʼ����ң�forward[i]�Ͷ�Ӧ�ڸò��next
    Node<K, V> *pre = this->head;
    // pre->forward[i]��Ӧÿһ��ĵ�һ���ڵ�
    for (int i = this->curLevel; i >= 0; i--)
    {
        while (pre->forward[i] != nullptr && pre->forward[i]->getKey() < key)
        {
            pre = pre->forward[i];
        }
        // ����ò����λ�õ�ǰ���ڵ��Դ�����
        update[i] = pre;
    }
    // ������0�㣬�Ҿ���while��pre���ǲ���λ�õ�ǰ���ڵ�
    Node<K, V> *insertPos = pre->forward[0];
    if (insertPos != nullptr && insertPos->getKey() == key)
    {
        mtx.unlock();
        // cout << "key : " << key << ", exists" << endl;
        return -1;
    }
    // insertPosΪ�ձ������ﵥ�����β�ڵ�
    if (insertPos == nullptr || insertPos->getKey() != key)
    {
        int randomLevel = this->getRandomLevel();
        if (randomLevel > this->curLevel)
        {
            // �����߲��ǰ���ڵ�����Ϊhead
            for (int i = this->curLevel + 1; i <= randomLevel; i++)
            {
                update[i] = this->head;
            }
            this->curLevel = randomLevel;
        }
        Node<K, V> *newNode = createNode(key, value, randomLevel);
        // �������Ͽ�ʼ�����룬����ʵֻ������һ���ڵ�
        // ֻ��ͨ��ָ��ʹ���߼���ִ��������
        for (int i = 0; i <= randomLevel; i++)
        {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }

        // cout << "insert successfully, key : " << key << ", value : " << value << endl;
        this->elemCount++;
    }
    mtx.unlock();
    return 0;
}

template <typename K, typename V>
void skipList<K, V>::displayList()
{
    cout << endl
         << "***** skipList *****" << endl;

    for (int i = 0; i <= this->curLevel; i++)
    {
        // ÿһ���һ���ڵ㿪ʼ
        Node<K, V> *node = this->head->forward[i];
        cout << "level " << i << ": ";
        while (node != nullptr)
        {
            cout << "(" << node->getKey() << " : " << node->getValue() << "); ";
            node = node->forward[i];
        }
        cout << endl;
    }
}

template <typename K, typename V>
void skipList<K, V>::clear()
{
    cout << "clear ..." << endl;
    Node<K, V> *node = this->head->forward[0];
    Node<K, V> *temp = nullptr;
    while (node != nullptr)
    {
        temp = node;
        node = node->forward[0];
        delete temp;
    }
    temp = nullptr;

    // ��ʼ��head
    for (int i = 0; i <= this->maxLevel; i++)
    {
        this->head->forward[i] = nullptr;
    }
    this->curLevel = 0;
    this->elemCount = 0;
}

template <typename K, typename V>
void skipList<K, V>::dumpFile()
{
    cout << "dumpFile ..." << endl;
    this->fileWriter.open(STORE_FILE);
    Node<K, V> *node = this->head->forward[0];

    while (node != nullptr)
    {
        this->fileWriter << node->getKey() << ":" << node->getValue() << endl;
        cout << node->getKey() << ":" << node->getValue() << ";" << endl;
        node = node->forward[0];
    }

    this->fileWriter.flush();
    this->fileWriter.close();
    return;
}

template <typename K, typename V>
void skipList<K, V>::loadFile()
{
    this->fileReader.open(STORE_FILE);
    cout << "loadFile ..." << endl;
    string line;
    string *key = new string();
    string *value = new string();
    while (getline(this->fileReader, line))
    {
        this->getKVFromString(line, key, value);
        if (key->empty() || value->empty())
        {
            continue;
        }
        insertElem(stoi(*key), *value);
        cout << "key : " << *key << "value : " << value << endl;
    }
    delete key;
    delete value;
    this->fileReader.close();
}

template <typename K, typename V>
void skipList<K, V>::getKVFromString(string &str, string *key, string *value)
{
    if (!this->isValidString(str))
    {
        return;
    }
    *key = str.substr(0, str.find(delimter));
    *value = str.substr(str.find(delimter) + 1, str.length());
}

template <typename K, typename V>
bool skipList<K, V>::isValidString(string &str)
{
    if (str.empty())
    {
        return false;
    }
    if (str.find(delimter) == string::npos)
    {
        return false;
    }
    return true;
}

template <typename K, typename V>
int skipList<K, V>::size()
{
    return this->elemCount;
}

template <typename K, typename V>
void skipList<K, V>::deleteElem(K key)
{
    mtx.lock();
    Node<K, V> *update[this->maxLevel + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (this->maxLevel + 1));

    Node<K, V> *pre = this->head;
    for (int i = this->curLevel; i >= 0; i--)
    {
        while (pre->forward[i] != nullptr && pre->forward[i]->getKey() < key)
        {
            pre = pre->forward[i];
        }
        update[i] = pre;
    }

    Node<K, V> *pDelete = pre->forward[0];
    if (pDelete != nullptr && pDelete->getKey() == key)
    {
        for (int i = 0; i <= this->curLevel; i++)
        {
            // �����ϲ��Ѿ�û��ָ��pDelete��ָ��
            if (update[i]->forward[i] != pDelete)
            {
                break;
            }
            update[i]->forward[i] = pDelete->forward[i];
        }
        // ��ɾ��Ԫ�������������
        while (this->curLevel > 0 && this->head->forward[this->curLevel] == nullptr)
        {
            this->curLevel--;
        }
        // cout << "successfully deleteElem" << endl;
        mtx.unlock();
        return;
    }
    else
    {
        // pDelete == nullptr || pDelete->getKey() != key
        // cout << "deleteElem failed, no such elem" << endl;
        mtx.unlock();
        return;
    }
}

template <typename K, typename V>
bool skipList<K, V>::searchElem(K key)
{
    // cout << "----searchElem----" << endl;
    Node<K, V> *pre = this->head;

    for (int i = this->curLevel; i >= 0; i--)
    {
        while (pre->forward[i] != nullptr && pre->forward[i]->getKey() < key)
        {
            pre = pre->forward[i];
        }
    }

    Node<K, V> *searchNode = pre->forward[0];
    if (searchNode != nullptr && searchNode->getKey() == key)
    {
        // cout << "found key: " << key << ", value : " << searchNode->getValue() << endl;
        return true;
    }
    else
    {
        // cout << "not found key: " << key << endl;
        return false;
    }
}

// �����ǰ�����ڣ�����ֵ����������ڣ�����flag�ж��Ƿ񴴽��ü�
// ����ֵ1��ʾ���³ɹ���0��ʾ�����ɹ���-1��ʾ����ʧ���Ҵ���ʧ��
template <typename K, typename V>
int skipList<K, V>::updateElem(K key, V value, bool needCreate)
{
    mtx1.lock();

    Node<K, V> *update[this->maxLevel + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (this->maxLevel + 1));

    Node<K, V> *pre = this->head;
    for (int i = this->curLevel; i >= 0; i--)
    {
        while (pre->forward[i] != nullptr && pre->forward[i]->getKey() < key)
        {
            pre = pre->forward[i];
        }
    }

    Node<K, V> *target = pre->forward[0];
    if (target != nullptr && target->getKey() == key)
    {
        // cout << "key : " << key << ", exists" << endl;
        // cout << "old value : " << target->getValue() << " --> ";
        target->setValue(value);
        // cout << "new value : " << target->getValue() << endl;
        mtx1.unlock();
        return 1;
    }
    // ��Ԫ�ز����ڣ�����Ҫ����
    if (needCreate)
    {
        this->insertElem(key, value);
        mtx1.unlock();
        return 0;
    }
    else
    {
        mtx1.unlock();
        return -1;
    }
}

template <typename K, typename V>
int skipList<K, V>::getRandomLevel()
{
    int level = 1;
    while (rand() % 2 == 0)
    {
        level++;
    }
    level = (level < this->maxLevel) ? level : this->maxLevel;
    return level;
}

template <typename K, typename V>
skipList<K, V>::skipList(int level)
{
    this->maxLevel = level;
    this->curLevel = 0;
    this->elemCount = 0;

    K key;
    V value;
    this->head = new Node<K, V>(key, value, level);
}

template <typename K, typename V>
skipList<K, V>::~skipList()
{
    if (this->fileWriter.is_open())
    {
        this->fileWriter.close();
    }
    if (this->fileReader.is_open())
    {
        this->fileReader.close();
    }
    delete this->head;
    this->head = nullptr;
}