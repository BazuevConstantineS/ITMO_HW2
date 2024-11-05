#include <iostream>
#include <random>
#include <vector>
#include <string>
#include <thread>
#include <dasmig/namegen.hpp>

int randomNumber(int range_in, int range_out);

class Client
{
public:

    bool gender; // true = male, false = female
    std::wstring name;
    std::wstring surname;
    int age;
    std::vector<std::string> items;
    int waiting_time;
    int cashNumber;

    Client() {
        gender = randomNumber(0, 1) == 0;
        name = (gender) ? dasmig::ng::instance().get_name(dasmig::ng::gender::m, dasmig::ng::culture::russian)
                        : dasmig::ng::instance().get_name(dasmig::ng::gender::f, dasmig::ng::culture::russian);
        surname = dasmig::ng::instance().get_surname(dasmig::ng::culture::russian);
        age = randomNumber(15, 80);
        generateItems(availableItems);
        waiting_time = randomNumber(1, 3);
    }

private:

    void generateItems(const std::vector<std::string>& availableItems) {
        int itemCount = randomNumber(1, 15);
        for (int i = 0; i < itemCount; ++i) {
            int randomIndex = randomNumber(0, availableItems.size() - 1);
            items.push_back(availableItems[randomIndex]);
        }
    };

    std::vector<std::string> availableItems = {
            "Milk", "Bread", "Eggs", "Butter", "Cheese",
            "Apples", "Bananas", "Chicken", "Pasta", "Rice",
            "Wine", "Beer", "Water", "Meat", "Spices"
    };

};

struct Node {

    Client client;
    Node* next;
    Node* prev;

    Node(const Client& client) : client(client), next(nullptr), prev(nullptr) {}
};

class DoublyLinkedList
{
public:

    DoublyLinkedList() : head(nullptr), tail(nullptr) {}

    void addClient(const Client& client) {

        Node* newNode = new Node(client);
        if (tail) {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        else {
            head = tail = newNode;
        }
        std::wcout << L"Client " << client.name << L" " << client.surname << L" has been added to the list.\n";
    }

    void removeFirstClient() {
        if (!head) return;

        Node* temp = head;
        head = head->next;
        if (head) {
            head->prev = nullptr;
        }
        else {
            tail = nullptr;
        }
        delete temp;
    }

    Client* getFirstClient() const {
        return head ? &head->client : nullptr;
    }

    bool isEmpty() const {
        return head == nullptr;
    }

    ~DoublyLinkedList() {
        while (!isEmpty())
        {
            removeFirstClient();
        }
    }

private:

    Node* head;
    Node* tail;

};

class CashRegister
{
public:

    CashRegister(DoublyLinkedList& clientList) : clients(clientList) {}

    void addClient(const Client& client) {
        clients.addClient(client);
    }

    void processNextClient() {
        if (clients.isEmpty()) {
            std::cout << "No clients in the queue.\n";
            return;
        }

        Client* currentClient = clients.getFirstClient();
        if (currentClient) {
            std::wcout << L"Processing client: " << currentClient->name << L" " << currentClient->surname << "\n";

            int processingTime = calculateProcessingTime(*currentClient);
            std::this_thread::sleep_for(std::chrono::seconds(processingTime));

            std::wcout << L"Client " << currentClient->name << L" " << currentClient->surname << L" finished checkout with "
                << currentClient->items.size() << L" items in " << processingTime << L" seconds.\n";
        }

        clients.removeFirstClient();
    }

    bool hasClients() const {
        return !clients.isEmpty();
    }

private:

    DoublyLinkedList& clients;

    int calculateProcessingTime(const Client& client) {
        int baseTime = randomNumber(1, 3);
        int itemTime = static_cast<int>(client.items.size());
        return baseTime + itemTime;
    }

};

int getNumber(const std::string& prompt, int min, int max);

int main()
{

    dasmig::ng::instance().load("C:/libs/name-generator-release-0.2/resources");  // path to recources for names

    int clientCount = getNumber("Enter the number of clients", 5, 15);

    int registerCount = getNumber("Enter the number of cash registers", 1, 5);

    DoublyLinkedList clientList;

    for (int i = 0; i < clientCount; ++i) {
        Client client;
        clientList.addClient(client);
    }

    std::vector<CashRegister> registers;
    for (int i = 0; i < registerCount; ++i) {
        registers.emplace_back(clientList);
    }

    std::vector<std::thread> registerThreads;
    for (int i = 0; i < registerCount; ++i) {
        registerThreads.emplace_back([&registers, i]() {
            while (registers[i].hasClients()) {
                registers[i].processNextClient();
            }
            });
    }

    for (auto& thread : registerThreads) {
        thread.join();
    }

    std::cout << "All clients have been processed.\n";

    return 0;

}

int randomNumber(int range_in, int range_out)
{
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(range_in, range_out); // define the range

    return distr(gen);
}

int getNumber(const std::string& prompt, int min, int max) {
    int choice;
    std::cout << prompt << " (Enter -1 for random): ";
    std::cin >> choice;
    if (choice == -1) {
        choice = randomNumber(min, max);
        std::cout << "Randomly selected: " << choice << std::endl;
    }
    return choice;
}