#include <algorithm>
#include <csignal>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

const int MAX_BOARD_SIZE = 32;
enum ERR { OK = 0, NOT_OK = 1 };

#define VERIFY_PREDICATE(predicate, errmsg) \
  do {                                      \
    if (!(predicate)) {                     \
      return ERR::NOT_OK;                   \
    }                                       \
  } while (0)

// ----------- DICTIONARY CLASS -----------
class Dictionary {
 public:
  bool IsWordValid(const std::string word) const;
  bool CanBeWord(const std::string str) const;
  ERR LoadDictionary(const char filename[]);

 private:
  std::set<std::string> m_dictionary;
  std::set<std::string> m_crazyDict;
};

ERR Dictionary::LoadDictionary(const char filename[]) {
  std::ifstream infile(filename);
  std::string line;
  while (std::getline(infile, line)) {
    m_dictionary.insert(line);
    for (int i = 1; i <= line.size(); ++i) {
      if (m_crazyDict.find(line.substr(0, i)) == m_crazyDict.end()) {
        m_crazyDict.insert(line.substr(0, i));
      }
    }
  }
  m_crazyDict.insert("");
  return ERR::OK;
}

bool Dictionary::IsWordValid(const std::string word) const {
  return m_dictionary.find(word) != m_dictionary.end();
}

bool Dictionary::CanBeWord(const std::string str) const {
  return m_crazyDict.find(str) != m_crazyDict.end();
}

// --------- BOGGLE GAME ------------
class BoggleGame {
 public:
  BoggleGame(const int size, const int wordSize, const char dictFile[],
             const std::vector<std::string>& boardVec);
  void FindWords();
  void PrintWordsAndScores();
  bool CheckAndLogLegalWord(std::string word);

 private:
  void FindWordsUtil(int ind, std::string str);
  void SortWords();
  void RemoveDuplicates();
  void ScoreWords();

  Dictionary m_dict;
  int m_squareSize;
  int m_minWordSize;
  int m_totalScore;
  std::vector<std::string> m_board;
  std::vector<bool> m_visited;
  std::vector<std::string> m_words;
  std::vector<int> m_wordScores;
};

BoggleGame::BoggleGame(const int size, const int wordSize,
                       const char dictFile[],
                       const std::vector<std::string>& boardVec)
    : m_squareSize(size), m_minWordSize(wordSize), m_totalScore(0) {
  m_board = boardVec;
  m_visited = std::vector<bool>(m_board.size(), false);
  m_dict.LoadDictionary(dictFile);
  std::cout << "--GAME BOARD--" << std::endl;
  for (int i = 0; i < m_board.size(); ++i) {
    std::cout << m_board[i] << " ";
    if ((i + 1) % m_squareSize == 0) {
      std::cout << std::endl;
    }
  }
  std::cout << "--------------" << std::endl;
}

void BoggleGame::FindWords() {
  std::string str = "";

  for (int i = 0; i < m_board.size(); ++i) {
    FindWordsUtil(i, str);
  }
  SortWords();
  RemoveDuplicates();
  ScoreWords();
}

void BoggleGame::FindWordsUtil(int ind, std::string str) {
  if (!m_dict.CanBeWord(str)) {
    return;
  }

  m_visited[ind] = true;
  str = str + m_board[ind];

  if (m_dict.IsWordValid(str) && str.length() >= m_minWordSize) {
    m_words.push_back(str);
  }

  int r = ind / m_squareSize;
  int c = ind % m_squareSize;

  for (int row = r - 1; row <= r + 1 && row < m_squareSize; ++row) {
    for (int col = c - 1; col <= c + 1 && col < m_squareSize; ++col) {
      if (row >= 0 && col >= 0) {
        int index = row * m_squareSize + col;
        if (!m_visited[index]) {
          FindWordsUtil(index, str);
        }
      }
    }
  }

  str.erase(str.length() - 1);
  m_visited[ind] = false;
}

void BoggleGame::PrintWordsAndScores() {
  std::cout << "Words found:" << std::endl;
  for (int i = 0; i < m_words.size(); ++i) {
    std::cout << m_words[i] << "\t";
    if (m_words[i].size() < 8) {
      std::cout << "\t";
    }
    std::cout << m_wordScores[i] << std::endl;
  }
  std::cout << "------------------------------------" << std::endl;
  std::cout << "Total number of words: " << m_words.size()
            << ", Total score: " << m_totalScore << std::endl;
}

void BoggleGame::SortWords() {
  std::sort(m_words.begin(), m_words.end(),
            [](const std::string& first, const std::string& second) {
              return first.size() > second.size();
            });
}

void BoggleGame::RemoveDuplicates() {
  std::vector<std::string> newList;
  for (int i = 0; i < m_words.size(); ++i) {
    bool foundDuplicate = false;
    for (int j = i + 1; j < m_words.size(); ++j) {
      if (m_words[i] == m_words[j]) {
        foundDuplicate = true;
      }
    }
    if (!foundDuplicate) {
      newList.push_back(m_words[i]);
    }
  }
  m_words = newList;
}

void BoggleGame::ScoreWords() {
  std::vector<int> wordScores;
  int totalScore = 0;
  int scoreCard[] = {0, 0, 0, 1, 1, 2, 3, 5, 11};
  for (int i = 0; i < m_words.size(); ++i) {
    int len = m_words[i].size();
    if (len > 8) {
      len = 8;
    }
    wordScores.push_back(scoreCard[len]);
    totalScore += scoreCard[len];
  }
  m_wordScores = wordScores;
  m_totalScore = totalScore;
}

bool BoggleGame::CheckAndLogLegalWord(std::string word) {
  std::vector<std::string>::iterator pos =
      std::find(m_words.begin(), m_words.end(), word);
  if (pos != m_words.end()) {
    std::cout << "YES, " << word << " is legal";
  } else {
    std::cout << "NO, " << word << " is illegal";
    if (m_dict.IsWordValid(word)) {
      std::cout << " in this game, but is a valid word!";
    }
  }
  std::cout << std::endl;
  return pos != m_words.end();
}

void signalHandler(int signum) {
  std::cout << std::endl << "Thanks for playing!" << std::endl;
  exit(signum);
}

int main() {
  signal(SIGINT, signalHandler);

  std::cout << "How big is the square you're playing on?" << std::endl;
  int size;
  std::cin >> size;

  int wordSize = 0;
  while (wordSize < 1) {
    std::cout << "What's the smallest allowable words size?" << std::endl;
    std::cin >> wordSize;
  }

  std::cout << "Input the board string separated by spaces:" << std::endl;
  std::string boardString;
  std::getline(std::cin >> std::ws, boardString);

  std::vector<std::string> board;
  size_t pos = 0;
  std::string token;
  while ((pos = boardString.find(" ")) != std::string::npos) {
    token = boardString.substr(0, pos);
    board.push_back(token);
    boardString.erase(0, pos + 1);
  }
  board.push_back(boardString);

  if (!(board.size() == size * size)) {
    std::cout << "Board length must be equal to the square of the size\n";
    return 1;
  }

  BoggleGame game(size, wordSize, "twl06.txt", board);
  game.FindWords();
  game.PrintWordsAndScores();

  std::string word;
  while (1) {
    std::cout << "Word to check: ";
    std::cin >> word;
    game.CheckAndLogLegalWord(word);
  }

  return 0;
}
