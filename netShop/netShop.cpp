// netShop.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <list>
#include <winsock2.h>
#include <regex>

#pragma comment(lib,"ws2_32.lib")
using namespace std;

//CHttp����������Http��Get���󣬲��һ�ȡ����˷��ص�����
class CHttp
{
public:
	CHttp();
	~CHttp();
	bool FetchGet(string url, string& htmlData);

private:
	string m_host;						//������
	string m_object;					//��Դ��
	bool m_bHTTPS;						//�Ƿ�ΪHTTPS
	unsigned int m_port = 80;			//�˿�
	SOCKET		m_socket;				//�ͻ����׽���

private:
	bool AnalyseURL(string url);
	bool InitSock();
	bool Connect();
};

CHttp::CHttp()
{
	m_bHTTPS = false;
}

CHttp::~CHttp()
{
	WSACleanup();
}

bool CHttp::InitSock()
{
	//��ʼ������
	WSADATA wd;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wd))
		return false;

	//�жϼ��ص��׽��ֿ�汾�Ƿ�һ��
	if (LOBYTE(wd.wVersion) != 2 || HIBYTE(wd.wVersion) != 2)
		return false;

	//�����׽���
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET)
		return false;

	return true;
}

bool CHttp::AnalyseURL(string url)
{
	//����URL
	if (string::npos == url.find("http://") && string::npos == url.find("https://"))
		return false;

	if (string::npos != url.find("http://"))
	{
		m_bHTTPS = false;
	}
	else
	{
		m_bHTTPS = true;
	}

	if (url.length() <= 8)
		return false;

	int pos = url.find('/', m_bHTTPS ? 8 : 7);
	if (pos == string::npos)
	{
		m_host = url.substr(m_bHTTPS ? 8 : 7);
		m_object = '/';
	}
	else
	{
		m_host = url.substr(m_bHTTPS ? 8 : 7, pos - (m_bHTTPS ? 8 : 7));
		m_object = url.substr(pos);
	}

	if (m_host.empty())
		return false;

	return true;
}

bool CHttp::Connect()
{
	//����IP��ַ
	hostent* host = gethostbyname(m_host.c_str());
	if (host == NULL)
		return false;

	//���ӷ�����
	sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(m_port);
	memcpy(&sa.sin_addr, host->h_addr, 4);
	if (SOCKET_ERROR == connect(m_socket, (sockaddr*)&sa, sizeof(sockaddr)))
		return false;

	return true;
}

bool CHttp::FetchGet(string url, string& htmlData)
{
	if (false == InitSock())
		return false;
	if (false == AnalyseURL(url))
		return false;
	if (false == Connect())
		return false;

	//����GET����
	//���ӣ�
	//GET http://desk.zol.com.cn/bizhi/8378_103862_2.html HTTP/1.1
	//Host: desk.zol.com.cn
	//Connection: keep-alive
	//Cache-Control: max-age=0
	//Upgrade-Insecure-Requests: 1
	//User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.87 Safari/537.36
	//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*; q = 0.8, application / signed - exchange; v = b3
	//Accept - Encoding: gzip, deflate
	//Accept - Language : zh - CN, zh; q = 0.9
	//Cookie: ip_ck = 5s6E4P / 2j7QuMTk1NzUxLjE1NzE1NTg1MjQ % 3D; z_pro_city = s_provice % 3Dzhejiang % 26s_city % 3Dhangzhou; userProvinceId = 26;
	//userCityId = 153; userCountyId = 0; userLocationId = 158648; lv = 1577597172; vn = 7; Hm_lvt_ae5edc2bc4fc71370807f6187f0a2dd0 = 1575991749, 1576802374; Adshow = 5; 
	//questionnaire_pv = 1577577603; Hm_lpvt_ae5edc2bc4fc71370807f6187f0a2dd0 = 1577597186; BDTUJIAID = 276a06702f0768d735c05fa98138038b

	string data;
	data = data + "GET " + m_object + " HTTP/1.1\r\n";
	//data = data + "HOST: " + m_host + "\r\n";
	data = data + "HOST: " + m_host + "\r\n";
	data = data + "Connection: close\r\n";
	data = data + "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/78.0.3904.87 Safari/537.36\r\n";
	data = data + "\r\n";

	if (SOCKET_ERROR == send(m_socket, data.c_str(), data.length(), 0))
		return false;

	//��������
	char ch = 0;	//��������
	int n = 0;		//ʵ�ʽ����ֽ���

	while ((n = recv(m_socket, &ch, sizeof(ch), 0)) > 0)
	{
		htmlData += ch;
		if (ch == ',')
			htmlData += '\n';
	}

	return true;
}

bool Request(string url, string& htmlData);
void regexMatchW(const string& reg);
void regexMatchD(const string& reg);

//���ﳵ�࣬���г�������Ʒ�������Ʒ��������Ʒ�ķ������Ƴ����ﳵ���������Ʒ�ġ�--������������أ���ʹ���˿������캯��������STL�е�List���洢���ﳵ�е���Ʒ
class Cart
{
public:
	Cart();
	Cart(const Cart& cpCart);
	friend class User;
	void listCart();
	void add();
	void clear();
	friend Cart operator--(Cart& TCart);
private:
	list<string> goodsInCart;
	string priceS;
	double priceD = 0;
};
//���ء�--������������Ƴ����ﳵ���������Ʒ
Cart operator--(Cart& TCart)
{
	if (TCart.goodsInCart.empty())
	{
		cout << "Cart is Empty!" << endl;
	}
	else
	{
		string goodsRemoveReq = "http://happybirthdaytolinan.site/php/price.php?" + TCart.goodsInCart.front();
		cout << "price:";
		Request(goodsRemoveReq, TCart.priceS);
		TCart.goodsInCart.pop_front();
		TCart.priceD -= stoi(TCart.priceS);
	}
	return TCart;
}

Cart::Cart()
{
	cout << "Your Cart is waiting for swallow!" << endl;
}

Cart::Cart(const Cart& cpCart)
{
	cout << "Cart is READY!" << endl;
}

//List�ı�����������ﳵ���������Ʒ
void Cart::listCart()
{
	list<string>::iterator it;
	for (it = goodsInCart.begin(); it != goodsInCart.end(); ++it)
		cout << *it << endl;
	cout << "total price:" << priceD << endl;
}

void Cart::add()
{
	string priceReq = "http://happybirthdaytolinan.site/php/price.php?";
	string goodsID, amount, goodsSold;
	cout << "Please Choose Goods'ID to add to cart:" << endl;
	cin >> goodsID;
	cout << "Please Enter Goods'amount:" << endl;
	cin >> amount;
	try
	{
		regexMatchD(goodsID);
		regexMatchD(amount);
	}
	catch (int)
	{
		cout << "goodsID or amount can't contain Charater!" << endl;
		return;
	}
	goodsInCart.push_front("goodsID=" + goodsID + "&amount=" + amount);
	cout << "price:";
	Request(priceReq + "goodsID=" + goodsID + "&amount=" + amount, priceS);
	priceD += stoi(priceS);
}

void Cart::clear()
{
	goodsInCart.clear();
	priceD = 0;
}

//User���࣬�����������û��ĸ��࣬����ʵ�����˹��ﳵ������һ�������࣬�л������г���Ʒ�����㹺�ﳵ����Ʒ����ȥ�����ﳵ����������Ʒ�Լ��г������˵���Ʒ���鿴���ķ���
class User
{
public:
	User(string N, string P);
	~User();
	Cart cart;
	string getUserPass();
	string getAccount();
	virtual void userLogined();
	friend void regexMatchW(const string& reg);
	friend void regexMatchD(const string& reg);

protected:
	string name, pass, returnData;
	int money;
	void listGoods();
	void getMoney();
	void check();
	void listBought();
	void removeTopGoods();
};

User::User(string N, string P)
{
	name = N;
	pass = P;
}

User::~User()
{
	cout << getAccount() << " log out!Looking forward to your next visit!" << endl;
}

void User::removeTopGoods()
{
	cart = --cart;
}

string User::getUserPass()
{
	return pass;
}

string User::getAccount()
{
	return name;
}

void User::check()
{
	string goodsSoldReq = "http://happybirthdaytolinan.site/php/goodsSold.php?";
	while (false == cart.goodsInCart.empty())
	{
		goodsSoldReq = goodsSoldReq + cart.goodsInCart.front() + "&UA=" + name;
		Request(goodsSoldReq, returnData);
		cart.goodsInCart.pop_front();
		cart.priceD = 0;
	}
}

void User::listBought()
{
	string userGoodsReq = "http://happybirthdaytolinan.site/php/userGoods.php?account=" + name;
	Request(userGoodsReq, returnData);
}

void User::listGoods()
{
	string listGoodsReq = "http://happybirthdaytolinan.site/php/goods.php";
	if (false == Request(listGoodsReq, returnData))
	{
		cout << "Request Failed,PLEASE check the network!" << endl;
		return;
	}
}

void User::getMoney()
{
	cout << "you have:";
	string moneyReq = "http://happybirthdaytolinan.site/php/money.php?account=" + name;
	Request(moneyReq, returnData);
	money = stoi(returnData);
	cout << "PLAESE Contact Alipay��13506871553 For Charge" << endl;
}

void User::userLogined()
{
	while (1)
	{
		char option = 'a';
		cout << "Welcome:" << getAccount() << endl;
		cout << "Press  a:ChooseGoods  b:ListCart  c:CheckALLGoods  d:ClearCart  e:RemoveTopGoods  f:ListMoney&Charge  \r\n\t  g:ListBought  else:back" << endl << "Please Enter: ";
		cin >> option;
		switch (option)
		{
		case 'a':listGoods(); cart.add(); break;
		case 'b':cart.listCart(); break;
		case 'c':check(); break;
		case 'd':cart.clear(); break;
		case 'e':removeTopGoods(); break;
		case 'f':getMoney(); break;
		case 'g':listBought(); break;
		default:
			return;
		}
	}
}

//VIP�û�������ͨ�û���࣬��¼������VIP����
class VIPUser :virtual public User
{
public:
	VIPUser(string N, string P) :User(N, P) { }
	virtual void userLogined();
private:
	int countRate = 95;
};

void VIPUser::userLogined()
{
	while (1)
	{
		char option = 'a';
		cout << "Welcome:" << " VIP " << getAccount() << endl;
		cout << "Press  a:ChooseGoods  b:ListCart  c:CheckALLGoods  d:ClearCart  e:RemoveTopGoods  f:ListMoney&Charge  \r\n\t  g:ListBought  else:back" << endl << "Please Enter: ";
		cin >> option;
		switch (option)
		{
		case 'a':listGoods(); cart.add(); break;
		case 'b':cart.listCart(); break;
		case 'c':check(); break;
		case 'd':cart.clear(); break;
		case 'e':removeTopGoods(); break;
		case 'f':getMoney(); break;
		case 'g':listBought(); break;
		default:
			return;
		}
	}
}

//����Ա�û���������Ʒ������ͨ�û�����VIP
class Admin :virtual public User
{
public:
	Admin(string N, string P) :User(N, P) { }
	virtual void userLogined();
protected:
	void addGoods();
	void deleteGoods();
	void ListAllSoldGoods();
	void deleteSoldGoodsByID();
	void deleteSoldGoodsByName();
	void setVip();
	void listUser();
};

void Admin::addGoods()
{
	string name, amount, price;
	cout << "Please Enter Goods Name:";
	cin >> name;
	cout << "Please Add Goods' amount:";
	cin >> amount;
	cout << "Please Add Goods' price:";
	cin >> price;
	try
	{
		regexMatchW(name);
		regexMatchD(amount);
		regexMatchD(price);
	}
	catch (int)
	{
		cout << "goods' price or amount can't contain Charater!" << endl;
		return;
	}
	catch (char)
	{
		cout << "goods' name can't contain Special Charater!" << endl;
		return;
	}
	string addGoodsReq = "http://happybirthdaytolinan.site/php/goodsAdd.php?name=" + name + "&amount=" + amount + "&price=" + price;
	if (false == Request(addGoodsReq, returnData))
	{
		cout << "Request Failed,PLEASE check the network!" << endl;
		return;
	}
}

void Admin::deleteGoods()
{
	string id;
	cout << "Please Enter Goods ID:";
	cin >> id;
	try
	{
		regexMatchD(id);
	}
	catch (int)
	{
		cout << "goods' ID can't contain Charater!" << endl;
		return;
	}
	string deleteGoodsReq = "http://happybirthdaytolinan.site/php/goodsDelete.php?goodsID=" + id;
	if (false == Request(deleteGoodsReq, returnData))
	{
		cout << "Request Failed,PLEASE check the network!" << endl;
		return;
	}
}

void Admin::ListAllSoldGoods()
{
	string ListAllSoldGoodsReq = "http://happybirthdaytolinan.site/php/userGoodsAll.php";
	if (false == Request(ListAllSoldGoodsReq, returnData))
	{
		cout << "Request Failed,PLEASE check the network!" << endl;
		return;
	}
}

void Admin::deleteSoldGoodsByID()
{
	string id;
	cout << "Please Enter GoodsSold ID:";
	cin >> id;
	try
	{
		regexMatchD(id);
	}
	catch (int)
	{
		cout << "goods' ID can't contain Charater!" << endl;
		return;
	}
	string soldGoodsDeleteByIDReq = "http://happybirthdaytolinan.site/php/soldGoodsDeleteByID.php?goodsSoldID=" + id;
	if (false == Request(soldGoodsDeleteByIDReq, returnData))
	{
		cout << "Request Failed,PLEASE check the network!" << endl;
		return;
	}
}

void Admin::deleteSoldGoodsByName()
{
	string name;
	cout << "Please Enter GoodsSold Name:";
	cin >> name;
	try
	{
		regexMatchW(name);
	}
	catch (char)
	{
		cout << "goods' name can't contain SPECIAL Charater!" << endl;
		return;
	}
	string soldGoodsDeleteByNameReq = "http://happybirthdaytolinan.site/php/soldGoodsDeleteByName.php?goodsName=" + name;
	if (false == Request(soldGoodsDeleteByNameReq, returnData))
	{
		cout << "Request Failed,PLEASE check the network!" << endl;
		return;
	}
}

void Admin::listUser()
{
	string listUserReq = "http://happybirthdaytolinan.site/php/allUser.php";
	if (false == Request(listUserReq, returnData))
	{
		cout << "Request Failed,PLEASE check the network!" << endl;
		return;
	}
}

void Admin::setVip()
{
	string account;
	cout << "Please Enter account:";
	cin >> account;
	try
	{
		regexMatchW(account);
	}
	catch (char)
	{
		cout << "Account's name can't contain SPECIAL Charater!" << endl;
		return;
	}
	string setVipReq = "http://happybirthdaytolinan.site/php/setVIP.php?account=" + account;
	if (false == Request(setVipReq, returnData))
	{
		cout << "Request Failed,PLEASE check the network!" << endl;
		return;
	}
}

void Admin::userLogined()
{
	while (1)
	{
		char option = 'a';
		cout << "Welcome:" << " Admin " << getAccount() << endl;
		cout << "Press  a:AddGoods  b:DeleteGoods  c:ListALLGoods  d:ListAllSoldGoods  e:DeleteSoldGoodsByID  \n  f:DeleteSoldGoodsByName   g:ListUser  h:SetVip  else:back" << endl << "Please Enter: ";
		cin >> option;
		switch (option)
		{
		case 'a':addGoods(); break;
		case 'b':deleteGoods(); break;
		case 'c':listGoods(); break;
		case 'd':ListAllSoldGoods(); break;
		case 'e':deleteSoldGoodsByID(); break;
		case 'f':deleteSoldGoodsByName(); break;
		case 'g':listUser(); break;
		case 'h':setVip(); break;
		default:
			return;
		}
	}
}

//�ռ�����Ա�����Ը��û���ֵ�����ó��ռ�����Ա���������Ȩ
class UltimateAdmin :public Admin, VIPUser
{
public:
	UltimateAdmin(string N, string P) :VIPUser(N, P), Admin(N, P), User(N, P) { }
	virtual void userLogined();
private:
	void chargeMoney();
	void setUser();
	void setAdmin();
	void listAccount();
	void deleteAccount();
};

void UltimateAdmin::chargeMoney()
{
	string account, amount;
	cout << "Please Enter Charge Account:";
	cin >> account;
	cout << "Please Enter Charge amount:";
	cin >> amount;
	try
	{
		regexMatchW(account);
		regexMatchD(amount);
	}
	catch (char)
	{
		cout << "Account's name can't contain SPECIAL Charater!" << endl;
		return;
	}
	catch (int)
	{
		cout << "Charge amount can't contain Charater!" << endl;
		return;
	}
	string addGoodsReq = "http://happybirthdaytolinan.site/php/charge.php?account=" + account + "&chargeAmount=" + amount;
	if (false == Request(addGoodsReq, returnData))
	{
		cout << "Request Failed,PLEASE check the network!" << endl;
		return;
	}
}

void UltimateAdmin::listAccount()
{
	string listAccountReq = "http://happybirthdaytolinan.site/php/allAccount.php";
	if (false == Request(listAccountReq, returnData))
	{
		cout << "Request Failed,PLEASE check the network!" << endl;
		return;
	}
}

void UltimateAdmin::setUser()
{
	string account;
	cout << "Please Enter account:";
	cin >> account;
	try
	{
		regexMatchW(account);
	}
	catch (char)
	{
		cout << "Account's name can't contain SPECIAL Charater!" << endl;
		return;
	}
	string setUserReq = "http://happybirthdaytolinan.site/php/setUser.php?account=" + account;
	if (false == Request(setUserReq, returnData))
	{
		cout << "Request Failed,PLEASE check the network!" << endl;
		return;
	}
}

void UltimateAdmin::setAdmin()
{
	string account;
	cout << "Please Enter account:";
	cin >> account;
	try
	{
		regexMatchW(account);
	}
	catch (char)
	{
		cout << "Account's name can't contain SPECIAL Charater!" << endl;
		return;
	}
	string setAdminReq = "http://happybirthdaytolinan.site/php/setAdmin.php?account=" + account;
	if (false == Request(setAdminReq, returnData))
	{
		cout << "Request Failed,PLEASE check the network!" << endl;
		return;
	}
}

void UltimateAdmin::deleteAccount()
{
	string account;
	cout << "Please Enter deleteAccount:";
	cin >> account;
	try
	{
		regexMatchW(account);
	}
	catch (char)
	{
		cout << "Account's name can't contain SPECIAL Charater!" << endl;
		return;
	}
	string deleteAccountReq = "http://happybirthdaytolinan.site/php/accountDelete.php?account=" + account;
	if (false == Request(deleteAccountReq, returnData))
	{
		cout << "Request Failed,PLEASE check the network!" << endl;
		return;
	}
}

void UltimateAdmin::userLogined()
{
	while (1)
	{
		char option = 'a';
		cout << "Welcome:" << " Ultimate Admin " << getAccount() << endl;
		cout << "Press  a:chargeMoney  b:setUser  c:setVIP  d:setAdmin  e:listAccount  f:deleteAccount  else:back" << endl << "Please Enter: ";
		cin >> option;
		switch (option)
		{
		case 'a':chargeMoney(); break;
		case 'b':setUser(); break;
		case 'c':setVip(); break;
		case 'd':setAdmin(); break;
		case 'e':listAccount(); break;
		case 'f':deleteAccount(); break;
		default:
			return;
		}
	}
}

void Welcome();
void Login(string& name, string& pass, string& returnData);
void Reg();

int main()
{
	string data, account, pass;
	Welcome();
	while (1)
	{
		char option = 'a';
		cout << "Please Login First Or Register" << endl;
		cout << "Press  a:Login \t b:Register \t Else:Exit \t Please Enter: ";
		cin >> option;
		if (option == 'a')
		{
			Login(account, pass, data);
		}
		else if (option == 'b')
		{
			Reg();
		}
		else
		{
			exit(0);
		}

		if (string::npos != data.find("Ultimate_Admin"))
		{
			User* UserPtr;
			UltimateAdmin UlAdmin(account, pass);
			UserPtr = &UlAdmin;
			UserPtr->userLogined();
		}
		else if (string::npos != data.find("VIP"))
		{
			User* UserPtr;
			VIPUser VIPuser(account, pass);
			UserPtr = &VIPuser;
			UserPtr->userLogined();
		}
		else if (string::npos != data.find("Admin"))
		{
			User* UserPtr;
			Admin AdminUser(account, pass);
			UserPtr = &AdminUser;
			UserPtr->userLogined();
		}
		else if (string::npos != data.find("welcome"))
		{
			User user(account, pass);
			user.userLogined();
		}
		account.clear();
		pass.clear();
		data.clear();
	}

	return 0;
}

//�û��ĵ�¼
void Login(string& account, string& password, string& returnData)
{
	cout << "Please Enter Your Account��" << endl;
	cin >> account;
	cout << "Please Enter Your Password��" << endl;
	cin >> password;
	try
	{
		regexMatchW(account);
		regexMatchW(password);
	}
	catch (char)
	{
		cout << "Account or Password can't contain special Charater!" << endl;
		return;
	}
	if (strlen(account.c_str()) < 6 || strlen(password.c_str()) < 6)
	{
		cout << "login FAILED, the length of account or password must more than 6!" << endl;
		return;
	}
	string loginReq = "http://happybirthdaytolinan.site/php/login.php?name=";
	loginReq = loginReq + account + "&pass=" + password;
	if (false == Request(loginReq, returnData))
	{
		cout << "Request Failed,PLEASE check the network!" << endl;
	}
}

//�û�ע��
void Reg()
{
	string account, password, returnData;
	cout << "Please Enter Your Account for Register ��" << endl;
	cin >> account;
	cout << "Please Enter Your Password for Register��" << endl;
	cin >> password;
	try
	{
		regexMatchW(account);
		regexMatchW(password);
	}
	catch (char)
	{
		cout << "Account or Password can't contain special Charater!" << endl;
		return;
	}
	if (strlen(account.c_str()) < 6 || strlen(password.c_str()) < 6)
	{
		cout << "register FAILED, the length of account or password must more than 6!" << endl;
		return;
	}
	string regReq = "http://happybirthdaytolinan.site/php/reg.php?name=";
	regReq = regReq + account + "&pass=" + password;
	if (false == Request(regReq, returnData))
	{
		cout << "Request Failed,PLEASE check the network!" << endl;
		return;
	}
}

//�û���ӭ����
void Welcome()
{
	string data;
	string noticeReq = "http://happybirthdaytolinan.site/php/notice.php?id=1";
	if (false == Request(noticeReq, noticeReq))
	{
		cout << "Request Failed,PLEASE check the network!" << endl;
		cout << "*****ATTENTION!*****" << endl << "Without Network you can't login or register!" << endl;
		return;
	}
	//system("pause");
	//system("cls");
}

//����Http��Get����ĺ���������ʵ������CHttp��Ķ��󣬲����ж������Ƿ�����
bool Request(string url, string& htmlData)
{
	//Get URL
	CHttp http;
	if (false == http.FetchGet(url, htmlData))
	{
		cout << "Get Request Failed!" << endl;
		return false;
	}
	htmlData = htmlData.substr(htmlData.find("[") + 1, htmlData.find("]") - htmlData.find("[") - 1);
	cout << htmlData << endl;

	return true;
}

//������ʽƥ�䵽�����ַ�ʱ�׳�"char"�����쳣
void regexMatchW(const string& regName)
{
	if (regex_search(regName, regex("\\W")))
		throw 'a';
}

//������ʽƥ�䵽������ʱ�׳�"int"�����쳣
void regexMatchD(const string& regDigi)
{
	if (regex_search(regDigi, regex("\\D")))
		throw 1;
}