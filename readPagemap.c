#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s [pid]\n", argv[0]);
        return 1;
    }
    
    char maps_filename[256];
    char pagemap_filename[256];
    int pid = atoi(argv[1]);
    
    // Open the maps file
    sprintf(maps_filename, "/proc/%d/maps", pid);
    FILE *maps_file = fopen(maps_filename, "r");
    if (maps_file == NULL) {
        printf("Error: failed to open file %s\n", maps_filename);
        return 1;
    }

    // Loop through each line in the maps file
    char line[256];
    while (fgets(line, sizeof(line), maps_file)) {
        unsigned long start, end;
        sscanf(line, "%lx-%lx", &start, &end);
        
        // Calculate the number of pages in the current memory region
        unsigned long num_pages = (end - start) / PAGE_SIZE;
        if ((end - start) % PAGE_SIZE != 0) {
            num_pages++;
        }
        
        // Open the pagemap file
        sprintf(pagemap_filename, "/proc/%d/pagemap", pid);
        int pagemap_fd = open(pagemap_filename, O_RDONLY);
        if (pagemap_fd < 0) {
            printf("Error: failed to open file %s\n", pagemap_filename);
            return 1;
        }
        
        // Read the pagemap entries for the current memory region
        unsigned long offset = start / PAGE_SIZE * sizeof(uint64_t);
        unsigned long i;
        for (i = 0; i < num_pages; i++) {
            uint64_t pagemap_entry;
            if (pread(pagemap_fd, &pagemap_entry, sizeof(uint64_t), offset) != sizeof(uint64_t)) {
                printf("Error: failed to read pagemap entry\n");
                return 1;
            }
            
            // Extract the page frame number (PFN) from the pagemap entry
            uint64_t pfn = pagemap_entry & 0x7fffffffffffffULL;
            if (pagemap_entry & (1ULL << 63)) {
                printf("Page present, ");
            } else {
                printf("Page not present, ");
            }
            printf("PFN: %llx\n", pfn);
            
            // Move to the next page in the memory region
            offset += sizeof(uint64_t);
        }
        
        // Close the pagemap file
        close(pagemap_fd);
    }
    
    // Close the maps file
    fclose(maps_file);
    
    return 0;
}
