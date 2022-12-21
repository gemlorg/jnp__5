
#ifndef FIFO_KVFIFO_H
#define FIFO_KVFIFO_H
#include <iostream>
#include <map>
#include <list>



//Celem tego zadania jest zaimplementowanie wzorca kontenera zachowującego
//się jak kolejka fifo, w której każdy element ma przyporządkowany klucz.
//Kontener ten powinien zapewniać silne gwarancje odporności na wyjątki oraz
//realizować semantykę kopiowania przy modyfikowaniu (ang. copy on write).



//Kopiowanie przy zapisie to technika optymalizacji szeroko stosowana
//m.in. w strukturach danych z biblioteki Qt oraz dawniej w implementacjach
//std::string. Podstawowa jej idea jest taka, że gdy tworzymy kopię obiektu
//(w C++ za pomocą konstruktora kopiującego lub operator przypisania), to
//        współdzieli ona wszystkie wewnętrzne zasoby (które mogą być przechowywane
//                                                     w oddzielnym obiekcie na stercie) z obiektem źródłowym. Taki stan trwa do
//momentu, w którym jedna z kopii musi zostać zmodyfikowana. Wtedy modyfikowany
//obiekt tworzy własną kopię owych zasobów, na których wykonuje modyfikacje.
//
//Wzorzec ma być parametryzowany typami klucza i przechowywanej wartości,
//oznaczanymi odpowiednio przez K i V. Typ klucza K ma semantykę wartości, czyli
//dostępne są dla niego bezparametrowy konstruktor domyślny, konstruktor
//        kopiujący, konstruktor przenoszący i operatory przypisania. Na typie K
//        zdefiniowany jest porządek liniowy i można na obiektach tego typu wykonywać
//wszelkie porównania. O typie V można jedynie założyć, że ma konstruktor
//kopiujący.
//
//W ramach tego zadania należy zaimplementować szablon
//

template <typename K, typename V> class kvfifo {

    class k_list {
    public:
        k_list * prev;
        k_list * next;
        const K key;
        V value;
        k_list(K key, V value) : key(key), value(value) {
            prev = this;
            next = this;
        }
    };
    using dict = std::map<K, std::list<k_list *>>;

public:
    //length of a queue
    uint64_t length;

    //pointer to the root of a linked list(allows to save the order of the queue + quick access to the first/last elem.
    k_list * order;

    //a key-value map. for each key we save all values assigned to this key in a list.
    //this allows quick search for elements by the key.
    dict tree;


    //Klasa kvfifo powinna udostępniać niżej opisane operacje. Przy każdej operacji
    //        podana jest jej oczekiwana złożoność czasowa przy założeniu, że nie trzeba
    //        wykonywać kopii. Oczekiwana złożoność czasowa operacji kopiowania przy zapisie
    //        wynosi O(n log n), gdzie n oznacza liczbę elementów przechowywanych w kolejce.
    //Wszystkie operacje muszą zapewniać co najmniej silną odporność na wyjątki,
    //        a konstruktor przenoszący i destruktor muszą być no-throw.


    //- Konstruktory: bezparametrowy tworzący pustą kolejkę, kopiujący i przenoszący.
    //Złożoność O(1).

    kvfifo() {
        length = 0;
        V v;
        K k;
        order = new k_list(v, k);
    }

    kvfifo(kvfifo const &) {

    }

    kvfifo(kvfifo &&) {

    }


    //- Operator przypisania przyjmujący argument przez wartość. Złożoność O(1) plus
    //        czas niszczenia nadpisywanego obiektu.
    kvfifo& operator=(kvfifo other) {

    }


    //- Metoda push wstawia wartość v na koniec kolejki, nadając jej klucz k.
    //Złożoność O(log n).

    void push(K const &k, V const &v) {
        //create a pointer
        k_list * o = new k_list(k, v);

        //map pointer+value to the key
        if(tree.find(k) != tree.end()) {
            tree.at(k).push_back(o);
        } else {
            tree.insert({k, {o}});
        }
//        std::cout << tree.at(k).back().first << std::endl;
        length++;

        //add reference to the value to the pointer and add the pointer to a linked list
        order_insert(o);


    }
    void order_insert(k_list * o) {
        o->next = order;
        o->prev = order->prev;
        order->prev->next = o;
        order->prev = o;
    }



    //- Metoda pop() usuwa pierwszy element z kolejki. Jeśli kolejka jest pusta, to
    //podnosi wyjątek std::invalid_argument. Złożoność O(log n).
    void pop() {
        K key = order->next->key;
        order_remove(order->next);
        tree.at(key).pop_front();
        length--;
    }
    void order_remove(k_list * k) {
        k->next->prev = k->prev;
        k->prev->next = k->next;
        free(k);
    }


    //- Metoda pop(k) usuwa pierwszy element o podanym kluczu z kolejki. Jeśli
    //        podanego klucza nie ma w kolejce, to podnosi wyjątek std::invalid_argument.
    //Złożoność O(log n).

    void pop(K const & key) {
        auto o = tree.at(key);
        order_remove(o.front());
        o.pop_front();
        length--;
    }

    //- Metoda move_to_back przesuwa elementy o kluczu k na koniec kolejki, zachowując
    //ich kolejność względem siebie. Zgłasza wyjątek std::invalid_argument, gdy
    //        elementu o podanym kluczu nie ma w kolejce. Złożoność O(m + log n), gdzie m to
    //        liczba przesuwanych elementów.
    void move_to_back(K const &k) {
        auto a = tree.at(k);
        for(k_list * l : a) {
            order_remove(l);
            order_insert(l);
        }

    }

    //- Metody front i back zwracają parę referencji do klucza i wartości znajdującej
    //się odpowiednio na początku i końcu kolejki. W wersji nie-const zwrócona para
    //powinna umożliwiać modyfikowanie wartości, ale nie klucza. Dowolna operacja
    //modyfikująca kolejkę może unieważnić zwrócone referencje. Jeśli kolejka jest
    //        pusta, to podnosi wyjątek std::invalid_argument. Złożoność O(1).

    std::pair<K const &, V &> front() {
        return {order->next->key,  order->next->value};

    }
    std::pair<K const &, V const &> front() const {
        return {order->next->key,  *order->next->value};
    }
    std::pair<K const &, V &> back() {
        return {order->prev->key, order->prev->value};
    }
    std::pair<K const &, V const &> back() const {
        return {order->prev->key, &*order->prev->value};
    }

    //- Metody first i last zwracają odpowiednio pierwszą i ostatnią parę
    //klucz-wartość o danym kluczu, podobnie jak front i back. Jeśli podanego klucza
    //        nie ma w kolejce, to podnosi wyjątek std::invalid_argument.
    //Złożoność O(log n).

    std::pair<K const &, V &> first(K const &key) {
        k_list * a = tree.at(key).front();
        return {a->key, a->value};
    }
    std::pair<K const &, V const &> first(K const &key) const {
        k_list * a = tree.at(key).front();
        return {a->key, *a->value};
    }
    std::pair<K const &, V &> last(K const &key) {
        k_list * a = tree.at(key).back();
        return {a->key, a->value};
    }
    std::pair<K const &, V const &> last(K const &key) const {
        k_list * a = tree.at(key).back();
        return {a->key, *a->value};
    }
    //
    //- Metoda size zwraca liczbę elementów w kolejce. Złożoność O(1).
    size_t size() const {
        return length;
    }

    //- Metoda empty zwraca true, gdy kolejka jest pusta, a false w przeciwnym
    //przypadku. Złożoność O(1).
    bool empty() const {
        return this->size() == 0;
    }

    //- Metoda count zwraca liczbę elementów w kolejce o podanym kluczu.
    //Złożoność O(log n).
    size_t count(K const & key) const {
        //check if element is in the tree.
        return tree.at(key).size();
    }

    //- Metoda clear usuwa wszystkie elementy z kolejki. Złożoność O(n).
    void clear() {
        for (auto  l : tree) {
            for(auto o : l) {
                order_remove(o);
            }
        }
        tree.clear();
        length = 0;
    }

    void print_queue() {
        while(!empty()) {
            std::cout << "key=" << front().first << " val=" << front().second << std::endl;
            pop();
        }
    }


    //- Iterator k_iterator oraz metody k_begin i k_end, pozwalające przeglądać zbiór
    //        kluczy w rosnącej kolejności ich wartości. Iteratory mogą być unieważnione przez
    //        dowolną zakończoną powodzeniem operację modyfikującą kolejkę oraz operacje
    //        front, back, first i last w wersjach bez const. Iterator musi spełniać koncept
    //std::bidirectional_iterator. Wszelkie operacje w czasie O(log n). Przeglądanie
    //        całej kolejki w czasie O(n). Iterator służy jedynie do przeglądania kolejki
    //i za jego pomocą nie można modyfikować kolejki, więc zachowuje się jak
    //        const_iterator z biblioteki standardowej.



    //
    //Tam gdzie jest to możliwe i uzasadnione należy opatrzyć metody kwalifikatorami
    //const i noexcept.
    //
    //Klasa kvfifo powinna być przezroczysta na wyjątki, czyli powinna przepuszczać
    //        wszelkie wyjątki zgłaszane przez wywoływane przez nią funkcje i przez operacje
    //na jej składowych, a obserwowalny stan obiektów nie powinien się zmienić.
    //W szczególności operacje modyfikujące zakończone niepowodzeniem nie powinny
    //unieważniać iteratorów.

};




#endif //FIFO_KVFIFO_H
