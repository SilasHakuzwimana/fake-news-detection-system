#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <algorithm>
#include <limits>
#include <fstream>
#include <sstream>

using namespace std;

struct NewsArticle
{
    int id;
    string title;
    string content;
    string source;
    string date;
    vector<string> keywords;

    NewsArticle() {}

    NewsArticle(int _id, const string &_title, const string &_content,
                const string &_source, const vector<string> &_keywords)
        : id(_id), title(_title), content(_content), source(_source), keywords(_keywords)
    {
        time_t now = time(0);
        date = ctime(&now);
        date.pop_back(); // remove newline
    }

    void display() const
    {
        cout << "\nID: " << id << "\nTitle: " << title << "\nContent: " << content
             << "\nSource: " << source << "\nDate: " << date << "\nKeywords: ";
        for (const auto &kw : keywords)
        {
            cout << kw << ", ";
        }
        cout << "\n";
    }
    void displayFormatted(int index) const
    {
        cout << "Article " << index << ": " << title << "\n";
        cout << "===============================\n";
        cout << "Content: "<< content << "\n";
        cout << "-------------------------------\n";
        cout << "Source: " << source << "\n";
        cout << "Date: " << date << "\n";
        cout << "Keywords: ";
        for (size_t i = 0; i < keywords.size(); ++i)
        {
            cout << keywords[i];
            if (i < keywords.size() - 1)
                cout << ", ";
        }
        cout << "\n=============================\n\n";
    }

    string serialize() const
    {
        stringstream ss;
        ss << id << "||" << title << "||" << content << "||" << source << "||" << date << "||";
        for (size_t i = 0; i < keywords.size(); ++i)
        {
            ss << keywords[i];
            if (i < keywords.size() - 1)
                ss << ",";
        }
        return ss.str();
    }

    static NewsArticle deserialize(const string &line)
    {
        stringstream ss(line);
        string token;
        NewsArticle article;
        getline(ss, token, '|');
        article.id = stoi(token);
        ss.ignore(1); // ignore second '|'
        getline(ss, article.title, '|');
        ss.ignore(1);
        getline(ss, article.content, '|');
        ss.ignore(1);
        getline(ss, article.source, '|');
        ss.ignore(1);
        getline(ss, article.date, '|');
        ss.ignore(1);
        getline(ss, token);
        stringstream kwstream(token);
        string kw;
        while (getline(kwstream, kw, ','))
        {
            article.keywords.push_back(kw);
        }
        return article;
    }
};

struct BSTNode
{
    NewsArticle article;
    BSTNode *left;
    BSTNode *right;

    BSTNode(const NewsArticle &a) : article(a), left(nullptr), right(nullptr) {}
};

vector<string> getKeywords()
{
    vector<string> keywords;
    string input;
    cout << "Enter keywords (comma separated): ";
    cin.ignore();
    getline(cin, input);
    stringstream ss(input);
    string kw;
    while (getline(ss, kw, ','))
    {
        kw.erase(remove_if(kw.begin(), kw.end(), ::isspace), kw.end());
        if (!kw.empty())
            keywords.push_back(kw);
    }
    return keywords;
}

class NewsBST
{
private:
    BSTNode *root;
    int nextId;

    BSTNode *insertHelper(BSTNode *node, const NewsArticle &article)
    {
        if (!node)
            return new BSTNode(article);
        if (article.id < node->article.id)
            node->left = insertHelper(node->left, article);
        else if (article.id > node->article.id)
            node->right = insertHelper(node->right, article);
        return node;
    }

    BSTNode *searchByIdHelper(BSTNode *node, int id) const
    {
        if (!node || node->article.id == id)
            return node;
        return id < node->article.id ? searchByIdHelper(node->left, id) : searchByIdHelper(node->right, id);
    }

    void inOrderHelper(BSTNode *node, vector<NewsArticle> &result) const
    {
        if (!node)
            return;
        inOrderHelper(node->left, result);
        result.push_back(node->article);
        inOrderHelper(node->right, result);
    }

    void searchByKeywordHelper(BSTNode *node, const string &keyword, vector<NewsArticle> &results) const
    {
        if (!node)
            return;
        searchByKeywordHelper(node->left, keyword, results);
        if (node->article.title.find(keyword) != string::npos ||
            find(node->article.keywords.begin(), node->article.keywords.end(), keyword) != node->article.keywords.end())
        {
            results.push_back(node->article);
        }
        searchByKeywordHelper(node->right, keyword, results);
    }

    BSTNode *deleteNodeHelper(BSTNode *node, int id)
    {
        if (!node)
            return node;
        if (id < node->article.id)
            node->left = deleteNodeHelper(node->left, id);
        else if (id > node->article.id)
            node->right = deleteNodeHelper(node->right, id);
        else
        {
            if (!node->left)
            {
                BSTNode *temp = node->right;
                delete node;
                return temp;
            }
            else if (!node->right)
            {
                BSTNode *temp = node->left;
                delete node;
                return temp;
            }
            BSTNode *temp = minValueNode(node->right);
            node->article = temp->article;
            node->right = deleteNodeHelper(node->right, temp->article.id);
        }
        return node;
    }

    BSTNode *minValueNode(BSTNode *node)
    {
        BSTNode *current = node;
        while (current && current->left)
            current = current->left;
        return current;
    }

public:
    NewsBST() : root(nullptr), nextId(1)
    {
        loadFromFile();
    }

    ~NewsBST()
    {
        saveToFile();
    }

    void addArticle(const string &title, const string &content, const string &source, const vector<string> &keywords)
    {
        NewsArticle article(nextId++, title, content, source, keywords);
        root = insertHelper(root, article);
        saveToFile();
        cout << "Article added with ID: " << article.id << endl;
    }

    NewsArticle *searchById(int id) const
    {
        BSTNode *node = searchByIdHelper(root, id);
        return node ? &node->article : nullptr;
    }

    vector<NewsArticle> searchByKeyword(const string &keyword) const
    {
        vector<NewsArticle> results;
        searchByKeywordHelper(root, keyword, results);
        return results;
    }

    vector<NewsArticle> getAllArticles() const
    {
        vector<NewsArticle> articles;
        inOrderHelper(root, articles);
        return articles;
    }

    bool updateArticle(int id)
    {
        NewsArticle* article = searchById(id);
        if (!article)
            return false;
    
        string title, content, source;
        vector<string> keywords;
    
        cin.ignore();
        cout << "Enter new title (or leave blank to keep current): ";
        getline(cin, title);
        if (!title.empty()) article->title = title;
    
        cout << "Enter new content (or leave blank to keep current): ";
        getline(cin, content);
        if (!content.empty()) article->content = content;
    
        cout << "Enter new source (or leave blank to keep current): ";
        getline(cin, source);
        if (!source.empty()) article->source = source;
    
        cout << "Do you want to update keywords? (y/n): ";
        char ch;
        cin >> ch;
        if (ch == 'y' || ch == 'Y') {
            keywords = getKeywords();
            article->keywords = keywords;
        }
    
        saveToFile(); 
        return true;
    }
    
    bool deleteArticle(int id)
    {
        if (!searchById(id))
            return false;
        root = deleteNodeHelper(root, id);
        saveToFile();
        return true;
    }

    void factCheck(const string &claim)
    {
        vector<string> keywords;
        stringstream ss(claim);
        string word;
        while (ss >> word)
        {
            if (word.length() > 3)
                keywords.push_back(word);
        }

        vector<NewsArticle> matches;
        for (const auto &kw : keywords)
        {
            vector<NewsArticle> res = searchByKeyword(kw);
            matches.insert(matches.end(), res.begin(), res.end());
        }

        sort(matches.begin(), matches.end(), [](const NewsArticle &a, const NewsArticle &b)
             { return a.id < b.id; });
        matches.erase(unique(matches.begin(), matches.end(), [](const NewsArticle &a, const NewsArticle &b)
                             { return a.id == b.id; }),
                      matches.end());

        if (matches.empty())
        {
            cout << "\nNo verified articles found. This claim might be fake.\n";
        }
        else
        {
            cout << "\nMatching articles found:\n";
            for (const auto &article : matches)
            {
                article.display();
                cout << "----------------------\n";
            }
        }
    }

    void saveToFile()
    {
        ofstream out("articles.txt");
        vector<NewsArticle> all = getAllArticles();
        int count = 1;
        for (const auto &article : all)
        {
            out << "Article " << count++ << ": " << article.title << "\n";
            out << "========================\n";
            out << "Content: "<<article.content << "\n";
            out << "------------------------\n";
            out << "Source: " << article.source << "\n";
            out << "Date: " << article.date << "\n";
            out << "Keywords: ";
            for (size_t i = 0; i < article.keywords.size(); ++i)
            {
                out << article.keywords[i];
                if (i < article.keywords.size() - 1)
                    out << ", ";
            }
            out << "\n========================\n\n";
        }
        out << "Total Articles: " << all.size() << "\n";
    }

    void loadFromFile()
    {
        ifstream in("articles.txt");
        string line;
        int maxId = 0;
        while (getline(in, line))
        {
            if (!line.empty())
            {
                NewsArticle article = NewsArticle::deserialize(line);
                root = insertHelper(root, article);
                maxId = max(maxId, article.id);
            }
        }
        nextId = maxId + 1;
    }
};

void displayMenu()
{
    cout << "\n";
    cout << "=========================================\n";
    cout << "||      Fake News Detection System     ||\n";
    cout << "=========================================\n";
    cout << "||  1. Add Verified Article            ||\n";
    cout << "||  2. Search Article by ID            ||\n";
    cout << "||  3. Search Articles by Keyword      ||\n";
    cout << "||  4. Fact-Check a Claim              ||\n";
    cout << "||  5. Display All Articles            ||\n";
    cout << "||  6. Update Article                  ||\n";
    cout << "||  7. Delete Article                  ||\n";
    cout << "||  8. Exit                            ||\n";
    cout << "=========================================\n";
    cout << "\nEnter your choice: ";
}

int main()
{
    NewsBST newsDB;
    int choice;

    do
    {
        displayMenu();
        while (!(cin >> choice))
        {
            cout << "Invalid input. Try again: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        switch (choice)
        {
        case 1:
        {
            string title, content, source;
            cin.ignore();
            cout << "\nEnter title: ";
            getline(cin, title);
            cout << "Enter content: ";
            getline(cin, content);
            cout << "Enter source: ";
            getline(cin, source);
            vector<string> keywords = getKeywords();
            newsDB.addArticle(title, content, source, keywords);
            break;
        }
        case 2:
        {
            int id;
            cout << "\nEnter article ID: ";
            cin >> id;
            NewsArticle *article = newsDB.searchById(id);
            if (article)
                article->display();
            else
                cout << "Article not found.\n";
            break;
        }
        case 3:
        {
            string keyword;
            cout << "Enter keyword: ";
            cin >> keyword;
            vector<NewsArticle> results = newsDB.searchByKeyword(keyword);
            if (results.empty())
                cout << "No articles found.\n";
            else
                for (auto &a : results)
                    a.display();
            break;
        }
        case 4:
        {
            string claim;
            cin.ignore();
            cout << "Enter the claim: ";
            getline(cin, claim);
            newsDB.factCheck(claim);
            break;
        }
        case 5:
        {
            vector<NewsArticle> articles = newsDB.getAllArticles();
            if (articles.empty())
                cout << "No articles.\n";
            else
            {
                int i = 1;
                for (auto &a : articles)
                    a.displayFormatted(i++);
            }
            break;
        } 

        case 6:
        {
          int id;
          cout << "Enter ID to update: ";
          cin >> id;
          if (newsDB.updateArticle(id))
              cout << "An article with ID: "<< id <<" updated successfully.\n";
          else
              cout << "An article with ID: "<< id <<" not found.\n";
          break;
        }
        case 7:
        {
            int id;
            cout << "Enter ID to delete: ";
            cin >> id;
            if (newsDB.deleteArticle(id))
                cout << "An article with ID: " <<id<<" deleted successfully!";
            else
                cout << "An article with ID: "<< id <<" not found.\n";
            break;
        }
        case 8:
            cout << "Goodbye!\n";
            break;
        default:
            cout << "Invalid option.\n";
        }
    } while (choice != 8);

    return 0;
}
