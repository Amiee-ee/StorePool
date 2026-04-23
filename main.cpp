#include<iostream>
using namespace std;
#include<vector>
#include<string>
#include<ctime>

//等大小划分的内存池
//先申请一大块内存 -> 分成一小块一小块(allocate)->用完后回收（deallocate)


//内存对齐
static inline size_t align_up(size_t n, size_t align) {
	return (n + (align - 1)) & ~(align - 1);
}

class StorePool {
public:
	//构造函数对成员变量初始化
	explicit StorePool(size_t blocks_number, size_t blocks_size) {
		blocks_number_ = blocks_number;
		blocks_size_ = adjust_blocks_size(blocks_size);
		List_ = nullptr;
	}

	//析构统一释放申请的内存，统一生命周期
	~StorePool() {
		for (void* p : pages_)
		{
			::operator delete[](p);
		}
	}

	//分配内存给对象
	void* allocate() {
		if (!List_) expand();
		ListNode* node = List_;
		List_ = node->next;
		return static_cast<void*>(node);

	}

	//回收内存
	void deallocate(void* p) {
		if (!p) return;
		ListNode* node = static_cast<ListNode*>(p);
		node->next = List_;
		List_ = node;
	}

	size_t blocks_number() const { return blocks_number_; }
	size_t blocks_size() const { return blocks_size_; }

private:


	//小块内存大小
	size_t adjust_blocks_size(size_t s) {
		size_t min = sizeof(void*);
		size_t cur = align_up(s < min ? min : s, alignof(void*));
		return cur;
	}

	//向系统申请一大块内存，并分成一块块小内存，用链表维护
	void expand() {
		size_t pages_size = blocks_number_ * blocks_size_;
		char* pages = static_cast<char*>(::operator new[](pages_size));
		pages_.push_back(pages);

		for (size_t i = 0; i < blocks_number_; i++) {
			char* addr = pages + i * blocks_size_;
			ListNode* node = reinterpret_cast<ListNode*>(addr);
			node->next = List_;
			List_ = node;
		}
	}

	//用链表维护内存
	struct ListNode
	{
		ListNode* next;
	};
	ListNode* List_;
	size_t blocks_number_;
	size_t blocks_size_;
	vector<void*> pages_;
};

//应用
struct Person {
	int age;
	/*string name;
	Person(int n,string nam):age(n),name(nam){}*/

	//年龄随机
	void c_age() {
		++age;
	}

	static void* operator new(size_t n);
	static void operator delete(void* p) noexcept;	//不允许异常
};

static StorePool sPerson(1024, sizeof(Person));

void* Person::operator new(size_t n) {
	return sPerson.allocate();
}

void Person::operator delete(void* p) noexcept {
	sPerson.deallocate(p);
}

int main() {
	vector<Person*> vec;
	int test_count = 2000;
	vec.reserve(test_count);
	cout << "开始分配 " << test_count << " 个 Person 对象..." << endl;

	for (int i = 0; i < test_count; i++) {
		Person* per = new Person{ i % 100 };
		cout << per->age << endl;
		vec.push_back(per);

		if (i % 500 == 0) {
			cout << "已分配 " << i + 1 << " 个对象" << endl;
		}
	}

	cout << "分配完成，开始释放..." << endl;

	for (auto* p : vec) {
		delete p;
	}

	vec.clear();

	//// 验证内存池正常工作
	cout << "\n额外测试：验证内存重用..." << endl;

	//// 重新分配一些对象，应该重用之前释放的内存
	for (int i = 0; i < 10; i++) {
		Person* per = new Person{ i * 10 };
		cout << "新对象 age: " << per->age << endl;
		delete per;  // 立即释放
	}

	cout << "\n测试完成！" << endl;

	return 0;

}