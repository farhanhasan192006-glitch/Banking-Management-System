// Banking Management System - OOPS Mini Project
// Prepared by: Farhan
// Compile: g++ -std=c++17 BankingManagementSystem.cpp -o BankingManagementSystem

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <iomanip>
#include <string>
#include <limits>
#include <stdexcept>

using namespace std;

class Account {
protected:
    int accountNumber;
    string holderName;
    double balance;

public:
    Account(int number, const string& name, double initialBalance)
        : accountNumber(number), holderName(name), balance(initialBalance) {
        if (initialBalance < 0)
            throw invalid_argument("Initial balance cannot be negative.");
    }

    virtual ~Account() = default;

    int getAccountNumber() const { return accountNumber; }
    string getHolderName() const { return holderName; }
    double getBalance() const { return balance; }

    virtual string getAccountType() const = 0;

    virtual void withdraw(double amount) {
        if (amount <= 0)
            throw invalid_argument("Withdrawal amount must be greater than zero.");
        if (amount > balance)
            throw runtime_error("Insufficient balance. Withdrawal cancelled.");
        balance -= amount;
    }

    void deposit(double amount) {
        if (amount <= 0)
            throw invalid_argument("Deposit amount must be greater than zero.");
        balance += amount;
    }

    virtual void display() const {
        cout << "\nAccount Number : " << accountNumber
             << "\nHolder Name    : " << holderName
             << "\nAccount Type   : " << getAccountType()
             << "\nAvailable Balance: Rs. " << fixed << setprecision(2) << balance
             << "\n";
    }
};

class SavingsAccount : public Account {
public:
    SavingsAccount(int number, const string& name, double initialBalance)
        : Account(number, name, initialBalance) {}

    string getAccountType() const override { return "Savings"; }

    void withdraw(double amount) override {
        // Savings account uses the common balance and amount checks.
        Account::withdraw(amount);
    }
};

class CurrentAccount : public Account {
public:
    CurrentAccount(int number, const string& name, double initialBalance)
        : Account(number, name, initialBalance) {}

    string getAccountType() const override { return "Current"; }

    void withdraw(double amount) override {
        // Current account also prevents withdrawals beyond available balance.
        Account::withdraw(amount);
    }
};

class TransactionLogger {
public:
    void log(int accountNumber, const string& type, double amount) const {
        ofstream file("transactions.txt", ios::app);
        if (!file)
            throw runtime_error("Unable to open transactions.txt.");
        file << accountNumber << " | " << type << " | Rs. "
             << fixed << setprecision(2) << amount << '\n';
    }
};

class Bank {
private:
    vector<unique_ptr<Account>> accounts;
    TransactionLogger logger;
    const string accountFile = "accounts.txt";

    Account* findAccount(int number) {
        for (auto& account : accounts) {
            if (account->getAccountNumber() == number)
                return account.get();
        }
        return nullptr;
    }

    void saveAccounts() const {
        ofstream file(accountFile);
        if (!file)
            throw runtime_error("Unable to save account records.");
        for (const auto& account : accounts) {
            file << account->getAccountType() << '|'
                 << account->getAccountNumber() << '|'
                 << account->getHolderName() << '|'
                 << fixed << setprecision(2) << account->getBalance() << '\n';
        }
    }

    void loadAccounts() {
        ifstream file(accountFile);
        if (!file) return; // First run: the file may not exist yet.

        string line;
        while (getline(file, line)) {
            string type, numberText, name, balanceText;
            stringstream ss(line);
            if (!getline(ss, type, '|') ||
                !getline(ss, numberText, '|') ||
                !getline(ss, name, '|') ||
                !getline(ss, balanceText, '|'))
                continue;

            try {
                int number = stoi(numberText);
                double amount = stod(balanceText);
                if (type == "Savings")
                    accounts.push_back(make_unique<SavingsAccount>(number, name, amount));
                else if (type == "Current")
                    accounts.push_back(make_unique<CurrentAccount>(number, name, amount));
            } catch (const exception&) {
                // Ignore a malformed record and continue loading other records.
            }
        }
    }

public:
    Bank() { loadAccounts(); }

    void createAccount() {
        int number, choice;
        string name;
        double amount;

        cout << "\nEnter account number: ";
        if (!(cin >> number)) throw invalid_argument("Account number must be numeric.");
        if (findAccount(number)) throw runtime_error("This account number already exists.");

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter account holder name: ";
        getline(cin, name);
        if (name.empty() || name.find('|') != string::npos)
            throw invalid_argument("Name cannot be empty or contain the | character.");

        cout << "1. Savings Account\n2. Current Account\nChoose account type: ";
        if (!(cin >> choice) || (choice != 1 && choice != 2))
            throw invalid_argument("Please choose 1 or 2.");

        cout << "Enter opening balance: Rs. ";
        if (!(cin >> amount)) throw invalid_argument("Balance must be numeric.");

        if (choice == 1)
            accounts.push_back(make_unique<SavingsAccount>(number, name, amount));
        else
            accounts.push_back(make_unique<CurrentAccount>(number, name, amount));

        saveAccounts();
        cout << "\nAccount created successfully!\n";
    }

    void depositMoney() {
        int number;
        double amount;
        cout << "\nEnter account number: ";
        if (!(cin >> number)) throw invalid_argument("Account number must be numeric.");
        Account* account = findAccount(number);
        if (!account) throw runtime_error("Account not found.");

        cout << "Enter deposit amount: Rs. ";
        if (!(cin >> amount)) throw invalid_argument("Amount must be numeric.");
        account->deposit(amount);
        logger.log(number, "Deposit", amount);
        saveAccounts();
        cout << "Deposit successful. New balance: Rs. "
             << fixed << setprecision(2) << account->getBalance() << '\n';
    }

    void withdrawMoney() {
        int number;
        double amount;
        cout << "\nEnter account number: ";
        if (!(cin >> number)) throw invalid_argument("Account number must be numeric.");
        Account* account = findAccount(number);
        if (!account) throw runtime_error("Account not found.");

        cout << "Enter withdrawal amount: Rs. ";
        if (!(cin >> amount)) throw invalid_argument("Amount must be numeric.");
        account->withdraw(amount);
        logger.log(number, "Withdrawal", amount);
        saveAccounts();
        cout << "Withdrawal successful. Remaining balance: Rs. "
             << fixed << setprecision(2) << account->getBalance() << '\n';
    }

    void displayAccount() {
        int number;
        cout << "\nEnter account number: ";
        if (!(cin >> number)) throw invalid_argument("Account number must be numeric.");
        Account* account = findAccount(number);
        if (!account) throw runtime_error("Account not found.");
        account->display(); // Runtime polymorphism through virtual display/type methods.
    }

    void displayAllAccounts() const {
        if (accounts.empty()) {
            cout << "\nNo accounts are available.\n";
            return;
        }
        cout << "\n========== ALL BANK ACCOUNTS ==========\n";
        for (const auto& account : accounts)
            account->display();
    }

    void run() {
        int choice;
        do {
            cout << "\n========== BANKING MANAGEMENT SYSTEM ==========\n"
                 << "1. Create Savings / Current Account\n"
                 << "2. Deposit Money\n"
                 << "3. Withdraw Money\n"
                 << "4. Display Account Details\n"
                 << "5. Display All Accounts\n"
                 << "0. Exit\n"
                 << "Enter your choice: ";

            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please enter a number.\n";
                continue;
            }

            try {
                switch (choice) {
                    case 1: createAccount(); break;
                    case 2: depositMoney(); break;
                    case 3: withdrawMoney(); break;
                    case 4: displayAccount(); break;
                    case 5: displayAllAccounts(); break;
                    case 0: cout << "\nThank you for using the Banking Management System!\n"; break;
                    default: cout << "Invalid menu choice. Try again.\n";
                }
            } catch (const exception& e) {
                cout << "Error: " << e.what() << '\n';
                if (cin.fail()) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            }
        } while (choice != 0);
    }
};

int main() {
    try {
        Bank bank;
        bank.run();
    } catch (const exception& e) {
        cerr << "Application error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
