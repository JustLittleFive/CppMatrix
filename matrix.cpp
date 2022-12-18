/*
 Copyright (c) 2022 SUSTech - JustLittleFive

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <string>

#include "fastMul.hpp"

// 提问:
// 我需要为Data模板类定义/声明构造函数与析构函数吗？Data类的内存可以由Matrix类管理吗？
// 如果不能，那么Data模板类可以放入另外一个模板类里面吗？(指Matrix类)
template <typename __T>
class Data {
 public:
  int used = 0;
  __T* num;
};

template <typename _T>
class Matrix {
 private:
  size_t row;
  size_t col;
  Data<_T>* dataptr = nullptr;

 public:
  size_t getCol() const { return this->col; }

  size_t getRow() const { return this->row; }

  Data<_T>* getData() const { return this->dataptr; }

  // 无参构造函数，构造一个空对象，不初始化Data对象
  Matrix<_T>() {
    this->row = 0;
    this->col = 0;
    this->dataptr = nullptr;
    return;
  }

  // 带尺寸参数的构造函数。dataptr指向一个Data对象。若参数不合法则仍然指向一个空对象
  Matrix<_T>(const int __row, const int __col) {
    this->row = __row;
    this->col = __col;
    if (this->row > 0 && this->col > 0) {
      this->dataptr = new Data<_T>();
      this->dataptr->num = new _T[row * col];
      this->dataptr->used = 1;
    } else {
      this->row = 0;
      this->col = 0;
      this->dataptr = nullptr;
    }
    return;
  }

  // 拷贝构造函数。以浅拷贝的方式拷贝数据：将dataptr指向已有的Data对象，该对象的used值+1
  Matrix<_T>(const Matrix<_T>& __temp) {
    this->row = __temp.row;
    this->col = __temp.col;
    if (this->row > 0 && this->col > 0) {
      this->dataptr = __temp.dataptr;
      this->dataptr->used++;
    } else {
      this->row = 0;
      this->col = 0;
      this->dataptr = nullptr;
    }
    return;
  }

  // 析构函数。
  ~Matrix() {
    if (this->dataptr != nullptr) {
      if (this->dataptr->used > 1) {
        this->dataptr->used--;
        this->dataptr = nullptr;
      } else {
        delete this->dataptr->num;
        this->dataptr->num = nullptr;
        this->dataptr->used = 0;
        this->dataptr = nullptr;
      }
      return;
    }
    return;
  }

  // 重载运算符+的函数模板(参数为常量)
  // 提问: 可以在模板类里面重载运算符吗？
  // 提问: 模板类的成员函数可以是函数模板吗？
  // Hint: 成员函数在模板类外实现时会出现模板参数不匹配的错误
  template <typename T>
  Matrix<_T> operator+(const T scalar) {
    if (this->row != 0 && this->col != 0) {
      for (int i = 0; i < row * col; i++) {
        this->dataptr->num[i] += (_T)scalar;
      }
      return *this;
    } else {
      std::cout << "add to empty matrix" << std::endl;
      return *this;
    }
  }

  // 重载运算符+的函数模板(参数为矩阵)
  template <typename T>
  Matrix<_T> operator+(const Matrix<T>& B) {
    if (this->row == B.getRow() && this->col == B.getCol() && this->row != 0 &&
        this->col != 0) {
      for (int i = 0; i < row * col; i++) {
        this->dataptr->num[i] += (_T)(B.getData()->num[i]);
      }
      return *this;
    } else {
      std::cout << "add to miss-matching matrix" << std::endl;
      return *this;
    }
  }

  // 重载运算符-的函数模板(参数为常量)
  template <typename T>
  Matrix<_T> operator-(const T scalar) {
    if (this->row != 0 && this->col != 0) {
      for (int i = 0; i < row * col; i++) {
        this->dataptr->num[i] -= (_T)scalar;
      }
      return *this;
    } else {
      std::cout << "minus from empty matrix" << std::endl;
      return *this;
    }
  }

  // 重载运算符-的函数模板(参数为矩阵)
  template <typename T>
  Matrix<_T> operator-(const Matrix<T>& B) {
    if (this->row == B.getRow() && this->col == B.getCol() && this->row != 0 &&
        this->col != 0) {
      for (int i = 0; i < row * col; i++) {
        this->dataptr->num[i] -= (_T)(B.getData()->num[i]);
      }
      return *this;
    } else {
      std::cout << "minus from miss-matching matrix" << std::endl;
      return *this;
    }
  }

  // 重载运算符*的函数模板(参数为常量)
  template <typename T>
  Matrix<_T> operator*(const T scalar) {
    if (this->row != 0 && this->col != 0) {
      for (int i = 0; i < row * col; i++) {
        this->dataptr->num[i] *= (_T)scalar;
      }
      return *this;
    } else {
      std::cout << "mul by empty matrix" << std::endl;
      return *this;
    }
  }

  // 重载运算符*的函数模板(参数为矩阵)，暂不支持不同类型矩阵相乘
  template <typename T>
  Matrix<_T> operator*(Matrix<T> B) {
    if (!this->row || !this->col || !B.getRow() || !B.getCol()) {
      std::cout << "mul by null matrix" << std::endl;
      return *this;
    } else if (this->col != B.getRow()) {
      std::cout << "mul by miss-matching matrix" << std::endl;
      return *this;
    } else {
      Matrix<_T> Temp(this->row, B.getCol());
      _T trans;
      int m = this->row;
      int n = B.getCol();
      int p = this->col;
      int lda = m;
      int ldb = n;
      int ldc = m;
      matmul_strassen(m, n, p, this->dataptr->num, lda, B.getData()->num, ldb,
                      Temp.dataptr->num, ldc);
      return Temp;
    }
    Matrix<_T> NullMatrix;
    return NullMatrix;
  }

  // 重载运算符==函数
  bool operator==(const Matrix<_T>& B) {
    if (this->col == B.getCol() && this->row == B.getRow()) {
      if (this->dataptr == B.getData()) {
        return true;
      }
      for (int i = 0; i < this->col * this->row; i++) {
        if (this->dataptr->num[i] != B.getData()->num[i]) {
          return false;
        }
      }
      return true;
    }
    return false;
  }

  // 重载运算符==函数模板，不同类型矩阵直接返回false
  template <typename T>
  bool operator==(const Matrix<T>& B) {
    return false;
  }

  // 同类型复制操作符=重载
  Matrix<_T>& operator=(const Matrix<_T>& B) {
    if (this->dataptr != nullptr) {
      if (this->dataptr->used > 1) {
        this->dataptr->used--;
        this->dataptr = nullptr;
      } else {
        delete this->dataptr->num;
        this->dataptr->num = nullptr;
        this->dataptr->used = 0;
        this->dataptr = nullptr;
      }
    }
    this->col = B.getCol();
    this->row = B.getRow();
    this->dataptr = B.getData();
    this->dataptr->used += 1;
    return *this;
  }

  // 不同类型赋值操作符=重载
  template <typename T>
  Matrix<_T>& operator=(const Matrix<T>& B) {
    if (this->dataptr != nullptr) {
      if (this->dataptr->used > 1) {
        this->dataptr->used--;
        this->dataptr = nullptr;
      } else {
        delete this->dataptr->num;
        this->dataptr->num = nullptr;
        this->dataptr->used = 0;
        this->dataptr = nullptr;
      }
    }
    this->dataptr = new Data<_T>();
    this->dataptr->num = new _T[this->row * this->col];
    this->dataptr->used = 0;
    this->col = B.getCol();
    this->row = B.getRow();
    for (int i = 0; i < this->col * this->row; i++) {
      this->dataptr->num[i] = (_T)B.getData()->num[i];
    }
    this->dataptr->used += 1;
    return *this;
  }

  // 矩阵赋值的成员函数，不允许使用不同的数据类型赋值
  // 用于赋值的数组的元素类型须与矩阵类型一致
  void setVal(_T* array, int len) {
    int realen = (len <= this->row * this->col) ? len : this->row * this->col;
    if (realen > 0 && this->dataptr != nullptr) {
      for (int i = 0; i < realen; i++) {
        this->dataptr->num[i] = array[i];
      }
      for (int i = realen; i < this->row * this->col; i++) {
        this->dataptr->num[i] = 0;
      }
    }
  }

  // 矩阵转置的成员函数
  Matrix<_T> Transpose() {
    Matrix<_T> temp(this->col, this->row);
    for (int i = 0; i < this->row; i++) {
      for (int j = 0; j < this->col; j++) {
        temp->dataptr->num[j * this->row + i] =
            this->dataptr->num[i * this->col + j];
      }
    }
    return temp;
  }

  // 适用于输入输出流的友元函数
  template <typename T>
  friend std::ostream& operator<<(std::ostream&, const Matrix<T>&);
  template <typename T>
  friend std::istream& operator>>(std::istream&, const Matrix<T>&);
};

// 之前的代码，试图在模板类外定义成员函数
// template <typename _T, typename T>
// Matrix<_T> Matrix<_T>::operator+(const T scalar) {
//   if (this->row != 0 && this->col != 0) {
//     for (int i = 0; i < row * col; ++i) {
//       this->dataptr->num[i] += (_T)scalar;
//     }
//     return *this;
//   } else {
//     Matrix<_T> NullMatrix(0, 0);
//     std::string WarningInformation = "add to empty matrix";
//     throw WarningInformation;
//     return NullMatrix;
//   }
// }

template <typename _T>
std::ostream& operator<<(std::ostream& strm, const Matrix<_T>& aim) {
  for (int i = 0; i < aim.row; i++) {
    for (int j = 0; j < aim.col; ++j)
      strm << aim.dataptr->num[i * aim.col + j]
           << ((char)(j == aim.col - 1) ? '\n' : ' ');
  }
  return strm;
}

template <typename _T>
std::istream& operator>>(std::istream& strm, const Matrix<_T>& aim) {
  for (int i = 0; i < aim.row; i++)
    for (int j = 0; j < aim.col; ++j) strm >> aim.dataptr->num[i * aim.col + j];
  return strm;
}

template <typename _T>
void printMatrix(const Matrix<_T>& m) {
  std::cout << "Matrix size: " << m.getRow() << " * " << m.getCol()
            << std::endl;
  if (m.getData() != nullptr) {
    std::cout << "Matrix be referenced: " << m.getData()->used << " times"
              << std::endl;
    std::cout << m;
  }
}

// 测试=重载的小测试
int main() {
  try {
    int array1[] = {1, 2, 3, 4, 5, 6};
    double array2[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6};
    Matrix<int> a(2, 3);
    printMatrix(a);

    a.setVal(array1, 6);
    printMatrix(a);

    Matrix<double> b(2, 3);
    b.setVal(array2, 6);
    printMatrix(b);

    Matrix<double> c(3, 2);
    c.setVal(array2, 6);
    printMatrix(c);

    a = b + a;
    printMatrix(a);
    printMatrix(b);

    b = b * c;
    printMatrix(b);

    Matrix<int> d(2, 2);
    d = a;
    printMatrix(a);
    printMatrix(d);
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
  }
}