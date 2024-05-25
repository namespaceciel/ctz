#include <iostream>
#include <string>

class func {
private:
    const std::string info = "Coroutine at level ";
    size_t flag            = 0;

public:
    std::string
    operator()() noexcept {
        switch (flag) {
            case 0 :
                return info + static_cast<char>('0' + flag++);

            case 1 :
                return info + static_cast<char>('0' + flag++);

            case 2 :
                return info + static_cast<char>('0' + flag++);

            case 3 :
                return info + static_cast<char>('0' + flag++);

            case 4 :
                return info + static_cast<char>('0' + flag++);

            case 5 :
                return info + static_cast<char>('0' + flag++);

            default :
                return "No more levels";
        }
    }

}; // struct func

int
main() {
    func f1;
    for (size_t i = 0; i < 3; ++i) {
        std::cout << f1() << '\n';
    }

    std::cout << '\n';

    func f2;
    for (size_t i = 0; i < 7; ++i) {
        std::cout << f2() << '\n';
    }
}
