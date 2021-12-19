#include <cstdint>
#include <thread>
#include <atomic>
#include "vector"
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <stdio.h>


// Gallery class. Contains all info about pictures, its viewers and fullness of exhibition.
class Gallery {
public:
    std::atomic<int> currently = 0;
    int maximum_at_once = 50;
    int pictures_amount = 0;
    std::vector<std::atomic<int>> pictures;

    Gallery(int at_once, int pictures_amount) {
        pictures = std::vector<std::atomic<int>> (pictures_amount);
        for (int i = 0; i < pictures_amount; ++i) {
            pictures[i] = 0;
        }
        this->pictures_amount = pictures_amount;
        maximum_at_once = at_once;
    }

};


// Visitor class. Behaves as impatient visitors -- tries to get to exhibition at all costs.
class Visitor {
public:
    Visitor() = default;

    void run(Gallery& g, int index, FILE * fp) {
        g.currently.load();
        fprintf(fp, "Visitor %d entered queue. \n", index);
        while (g.maximum_at_once <= g.currently) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            g.currently.load();
        }
        g.currently.fetch_add(1);
        fprintf(fp, "Visitor %d entered exhibition. Currently %d visitors. \n", index, (int)g.currently);
        // Every visitor wants to have a look of 5 pictures after what he leaves
        for (int i = 0; i < 5; ++i) {
            int r = std::rand() % g.pictures_amount;
            fprintf(fp, "Visitor %d wants to see picture %d \n", index, r);

            g.pictures[i].load();
            if (g.pictures[r] < 10) {
                fprintf(fp, "Visitor %d found a place to see picture %d \n", index, r);
                g.pictures[r].fetch_add(1);
                // Watching picture
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                // End of watch
                g.pictures[r].fetch_sub(1);
                fprintf(fp, "Visitor %d left picture %d \n", index, r);
                continue;
            } else {
                // Picture is full. Waiting for free space
                fprintf(fp, "Visitor %d didn't find a place to see picture %d. Waiting\n", index, r);
                while (g.pictures[r] >= 10) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    g.pictures[r].load();
                }

                // Found a place to watch picture
                fprintf(fp, "Visitor %d found a place to see picture %d \n", index, r);
                g.pictures[r].fetch_add(1);
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                g.pictures[r].fetch_sub(1);
                fprintf(fp, "Visitor %d left picture %d\n", index, r);
            }
        }
        g.currently.fetch_sub(1);
        fprintf(fp, "Visitor %d left exhibition\n", index);
    }
};

void process(Visitor& user, Gallery& g, int x, FILE * fp) {
    user.run(g, x, fp);
}


int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Wrong input!\n"
               "Use this hint:\n"
               "<output_file> <total_visitors_amount> <maximum_visitors_amount> <pictures_amount>");
        return -1;
    }

    std::string filename = argv[1];
    int visitors_amount, max_at_once, pictures_amount;
    visitors_amount = std::stoi(argv[2]);

    FILE * pFile = fopen (filename.data(),"w");
    max_at_once = std::stoi(argv[3]);
    pictures_amount = std::stoi(argv[4]);

    std::vector<std::thread> threads (visitors_amount);
    Gallery g (max_at_once, pictures_amount);
    std::vector<Visitor> visitors (visitors_amount);

    for (int i = 0; i < visitors_amount; ++i) {
        threads[i] = std::thread(process, std::ref(visitors[i]), std::ref(g), i, pFile);
    }

    for (int i = 0; i < visitors_amount; ++i) {
        threads[i].join();
    }

    return 0;
}