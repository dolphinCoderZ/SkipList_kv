#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <fstream>
#include <mutex>

#define STORE_FILE "store/dump_file"
using namespace std;

// 临界区锁
mutex mtx, mtx1;
// 分隔符
string delimter = ":";

// 跳表节点
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

    // 对于不同层，可能指向多个节点，指针数组保存
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

    // 包含各层指向下一节点的指针
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
    // 随机产生层数
    int getRandomLevel();

    Node<K, V> *createNode(K key, V value, int level);
    // 增删改查
    int insertElem(K key, V value);
    int updateElem(K key, V value, bool needCreate = false);
    bool searchElem(K key);
    void deleteElem(K key);
    // 数据落盘和数据加载
    void dumpFile();
    void loadFile();

    void displayList();
    void clear();
    int size();

private:
    void getKVFromString(string &str, string *key, string *value);
    bool isValidString(string &str);

private:
    // 跳表的最大层数
    int maxLevel;
    // 当前跳表的最高层
    int curLevel;

    Node<K, V> *head;
    ofstream fileWriter;
    ifstream fileReader;
    // 跳表的元素个数
    int elemCount;
};

template <typename K, typename V>
Node<K, V> *skipList<K, V>::createNode(const K key, const V value, int level)
{
    Node<K, V> *node = new Node<K, V>(key, value, level);
    return node;
}

template <typename K, typename V>
// 返回值：-1表示元素存在，0表示插入成功
int skipList<K, V>::insertElem(K key, V value)
{
    mtx.lock();
    // 拥有最多指针的节点，其指针数组的数目最大是maxLevel+1
    Node<K, V> *update[this->maxLevel + 1];
    memset(update, 0, sizeof(Node<K, V> *) * (this->maxLevel + 1));
    // 逐层查找
    // 从每一层的第一个节点开始向后找，forward[i]就对应于该层的next
    Node<K, V> *pre = this->head;
    // pre->forward[i]对应每一层的第一个节点
    for (int i = this->curLevel; i >= 0; i--)
    {
        while (pre->forward[i] != nullptr && pre->forward[i]->getKey() < key)
        {
            pre = pre->forward[i];
        }
        // 保存该层插入位置的前驱节点以待更新
        update[i] = pre;
    }
    // 遍历到0层，且经过while，pre就是插入位置的前驱节点
    Node<K, V> *insertPos = pre->forward[0];
    if (insertPos != nullptr && insertPos->getKey() == key)
    {
        mtx.unlock();
        // cout << "key : " << key << ", exists" << endl;
        return -1;
    }
    // insertPos为空表明到达单链表的尾节点
    if (insertPos == nullptr || insertPos->getKey() != key)
    {
        int randomLevel = this->getRandomLevel();
        if (randomLevel > this->curLevel)
        {
            // 将增高层的前驱节点设置为head
            for (int i = this->curLevel + 1; i <= randomLevel; i++)
            {
                update[i] = this->head;
            }
            this->curLevel = randomLevel;
        }
        Node<K, V> *newNode = createNode(key, value, randomLevel);
        // 从下往上开始逐层插入，但其实只构造了一个节点
        // 只是通过指针使其逻辑上执行逐层插入
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
        // 每一层第一个节点开始
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

    // 初始化head
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
            // 在往上层已经没有指向pDelete的指针
            if (update[i]->forward[i] != pDelete)
            {
                break;
            }
            update[i]->forward[i] = pDelete->forward[i];
        }
        // 若删除元素引起层数减少
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

// 如果当前键存在，更新值；如果不存在，根据flag判断是否创建该键
// 返回值1表示更新成功，0表示创建成功，-1表示更新失败且创建失败
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
    // 若元素不存在，但需要创建
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