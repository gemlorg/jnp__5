

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

    class k_node {
    public:
        k_node * prev;
        k_node * next;
        const K key;
        V value;
        k_node(K key, V value) : key(key), value(value) {
            prev = this;
            next = this;
        }
    };
private:
    using dict = std::map<K, std::list< k_node * >>;
    //length of a queue
    uint64_t length;

    //pointer to the root of a linked list(allows to save the order of the queue + quick access to the first/last elem.
    std::shared_ptr<k_node *>  order;

    //a key-value map. for each key we save all values assigned to this key in a list.
    //this allows quick search for elements by the key.
    std::shared_ptr<dict> tree;
    bool can_be_modified;

public:

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
        k_node * p = new k_node(v, k);
        order = std::make_shared<k_node *>(p);
        tree = std::make_shared<dict>(dict{});
    }

    kvfifo(kvfifo const & other) {
        length = other.length;
        tree = other.tree;
        order = other.order;
        if(other.can_be_modified) {
            full_copy();
        }
    }

    kvfifo(kvfifo && other) noexcept {
        length = other.length;
        tree = other.tree;
        order = other.order;
        other.length = 0;
        other.tree = nullptr;
        other.order = nullptr;
    }


    //- Operator przypisania przyjmujący argument przez wartość. Złożoność O(1) plus
    //        czas niszczenia nadpisywanego obiektu.
    kvfifo& operator=( kvfifo other) {
        std::cout << "other is: " << other.length;
        if(this != &other) {

            struct k_node* head = (*order)->next;
            struct k_node* tmp ;
            while (length > 0) {
                tmp = head;
                head = head->next;
                length--;
                delete tmp;
            }
            delete head;
//            std::cout << other.size();
            length = other.size();
            tree = other.tree;
            order = other.order;
            if(other.can_be_modified) {
                full_copy();
            }

        }
        return *this;
    }
    V& operator=( V  other) {
        copy_on_write();
    }


    //- Metoda push wstawia wartość v na koniec kolejki, nadając jej klucz k.
    //Złożoność O(log n).

    void push(K const &k, V const &v) {
        can_be_modified = false;
        copy_on_write();
        //create a pointer
        k_node * o = new k_node(k, v);

        //map pointer+value to the key
        if(tree->find(k) != tree->end()) {
            tree->at(k).push_back(o);
        } else {
            tree->insert({k, {o}});
        }
//        std::cout << tree->at(k).back()->value << std::endl;

        length++;

        //add reference to the value to the pointer and add the pointer to a linked list
        order_insert(o);


    }
    void copy_on_write() {
        if(tree.use_count() > 1) {
            full_copy();
        }
    }
    void full_copy() {
        dict a{};

        k_node * root = new k_node((*order)->key, (*order)->value);
        k_node * tmp = (*order)->next;
        while(tmp != *order) {
            k_node * o = new k_node(tmp->key, tmp->value);
            if(a.find(tmp->key) != a.end()) {
                a.at(tmp->key).push_back(o);
            } else {
                a.insert({tmp->key, {o}});
            }
            o->next = root;
            o->prev = root->prev;
            root->prev->next = o;
            root->prev = o;
            tmp = tmp->next;
        }
        tree = std::make_shared<dict>(a);
        order = std::make_shared<k_node * >(root);


    }
    void order_insert(k_node * o) {
        o->next = *order;
        o->prev = (*order)->prev;
        (*order)->prev->next = o;
        (*order)->prev = o;
    }



    //- Metoda pop() usuwa pierwszy element z kolejki. Jeśli kolejka jest pusta, to
    //podnosi wyjątek std::invalid_argument. Złożoność O(log n).
    void pop() {
        can_be_modified = false;
        copy_on_write();
        K key = (*order)->next->key;
        order_remove((*order)->next);
        if(tree->at(key).size() > 1) {
            tree->at(key).pop_front();
        } else {
            tree->erase(key);
        }
        length--;
    }
    void order_remove(k_node * k) {
        k->next->prev = k->prev;
        k->prev->next = k->next;
        free(k);
    }
    void order_temp_remove(k_node * k) {
        k->prev->next = k->next;
        k->next->prev = k->prev;
    }



    //- Metoda pop(k) usuwa pierwszy element o podanym kluczu z kolejki. Jeśli
    //        podanego klucza nie ma w kolejce, to podnosi wyjątek std::invalid_argument.
    //Złożoność O(log n).

    void pop(K const & key) {
        can_be_modified = false;
        copy_on_write();
        order_remove(tree->at(key).front());
        if(tree->at(key).size() > 1) {
            tree->at(key).pop_front();
        } else {
            tree->erase(key);
        }


        length--;
    }



    //- Metoda move_to_back przesuwa elementy o kluczu k na koniec kolejki, zachowując
    //ich kolejność względem siebie. Zgłasza wyjątek std::invalid_argument, gdy
    //        elementu o podanym kluczu nie ma w kolejce. Złożoność O(m + log n), gdzie m to
    //        liczba przesuwanych elementów.
    void move_to_back(K const &k) {
        can_be_modified = false;
        copy_on_write();
        auto o = tree->at(k);
        for(k_node * l : o) {
            order_temp_remove(l);
        }
        for(k_node * l : o) {
            order_insert(l);
        }

    }

    //- Metody front i back zwracają parę referencji do klucza i wartości znajdującej
    //się odpowiednio na początku i końcu kolejki. W wersji nie-const zwrócona para
    //powinna umożliwiać modyfikowanie wartości, ale nie klucza. Dowolna operacja
    //modyfikująca kolejkę może unieważnić zwrócone referencje. Jeśli kolejka jest
    //        pusta, to podnosi wyjątek std::invalid_argument. Złożoność O(1).

    std::pair<K const &, V &> front() {
        copy_on_write();
        can_be_modified = true;
        return {(*order)->next->key,  (*order)->next->value};

    }
    std::pair<K const &, V const &> front() const {
        return {(*order)->next->key, (*order)->next->value};
    }
    std::pair<K const &, V &> back() {
        copy_on_write();
        can_be_modified = true;
        return {(*order)->prev->key, (*order)->prev->value};
    }
    std::pair<K const &, V const &> back() const {
        return {(*order)->prev->key, (*order)->prev->value};
    }

    //- Metody first i last zwracają odpowiednio pierwszą i ostatnią parę
    //klucz-wartość o danym kluczu, podobnie jak front i back. Jeśli podanego klucza
    //        nie ma w kolejce, to podnosi wyjątek std::invalid_argument.
    //Złożoność O(log n).

    std::pair<K const &, V &> first(K const &key) {
        copy_on_write();
        can_be_modified = true;
        k_node * a = tree->at(key).front();

        return {a->key, a->value};
    }
    std::pair<K const &, V const &> first(K const &key) const {
        k_node * a = tree->at(key).front();
        return {a->key, a->value};
    }
    std::pair<K const &, V &> last(K const &key) {
        copy_on_write();
        can_be_modified = true;
        k_node * a = tree->at(key).back();
        return {a->key, a->value};
    }
    std::pair<K const &, V const &> last(K const &key) const {
        k_node * a = tree->at(key).back();
        return {a->key, a->value};
    }
    //
    //- Metoda size zwraca liczbę elementów w kolejce. Złożoność O(1).
    size_t size() const {
        return length;
    }

    //- Metoda empty zwraca true, gdy kolejka jest pusta, a false w przeciwnym
    //przypadku. Złożoność O(1).
    bool empty() const {
        return length == 0;
    }

    //- Metoda count zwraca liczbę elementów w kolejce o podanym kluczu.
    //Złożoność O(log n).
    size_t count(K const & key) const {
        //check if element is in the tree.
        if(tree->find(key) == tree->end()) {
            return 0;
        }
        return tree->at(key).size();
    }

    //- Metoda clear usuwa wszystkie elementy z kolejki. Złożoność O(n).
    void clear() {
        for (auto  l : *tree) {
            for(auto o : l) {
                order_remove(o);
            }
        }
        tree->clear();
        length = 0;
    }

    void print_queue() {
        std::cout << "currently watching: " << tree.use_count() << std::endl;
        std::cout << "Dict print: " << std::endl;
        for (auto it = (*tree).begin(); it != (*tree).end(); ++it) {
            std::cout << it->first << " vals: ";
            for (auto it1: it->second) {
                std::cout << it1->value << " ";
            }
        }
        std::cout << "\nQueue print: " << std::endl;
        auto * tmp = (*order)->next;
        for(int i = 0 ; i < length; i++) {
            std::cout << "key=" << tmp->key << " val=" << tmp->value << std::endl;
            tmp = tmp->next;
        }
        std::cout << std::endl;
    }


    //- Iterator k_iterator oraz metody k_begin i k_end, pozwalające przeglądać zbiór
    //        kluczy w rosnącej kolejności ich wartości. Iteratory mogą być unieważnione przez
    //        dowolną zakończoną powodzeniem operację modyfikującą kolejkę oraz operacje
    //        front, back, first i last w wersjach bez const. Iterator musi spełniać koncept
    //std::bidirectional_iterator. Wszelkie operacje w czasie O(log n). Przeglądanie
    //        całej kolejki w czasie O(n). Iterator służy jedynie do przeglądania kolejki
    //i za jego pomocą nie można modyfikować kolejki, więc zachowuje się jak
    //        const_iterator z biblioteki standardowej.

    class k_iterator {
    private:
      typename dict::const_iterator key_iterator;
      //k_iterator_t key_iterator;

    public:
      using iterator_category = std::bidirectional_iterator_tag;;
      using value_type = K;
    	using difference_type = ptrdiff_t;
    	using pointer = value_type*;
   	  using reference = const value_type&;

      k_iterator() = default;

      k_iterator(typename dict::const_iterator key_iterator_) : key_iterator(key_iterator_){};

      reference operator*() const noexcept {
        return (*key_iterator).first;
      }
      pointer operator->() const noexcept {
      	return *(*key_iterator).first;
   	  }
    	k_iterator & operator++() noexcept {  // ++it
      	key_iterator++;
        return *this;
   	  }
      k_iterator operator++(int) noexcept { // it++
        k_iterator result(*this);
        operator++();
      	return result;
   	  }
   	  k_iterator & operator--() noexcept {  // --it
      	key_iterator--;
        return *this;
   	  }
      k_iterator operator--(int) noexcept { // it--
        k_iterator result(*this);
        operator--();
      	return result;
   	  }

    	friend bool operator==(k_iterator const & a,
                             k_iterator const & b
      ) noexcept {
        return a.key_iterator == b.key_iterator;
      }
      friend bool operator!=(k_iterator const & a,
                             k_iterator const & b
      ) noexcept {
      	return !(a == b);
      }

    };

    k_iterator k_begin() const noexcept {
        return k_iterator(tree->begin());
    }

    k_iterator k_end() const noexcept {
        return k_iterator(tree->end());
    }

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
