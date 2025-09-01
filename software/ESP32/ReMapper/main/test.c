#include <stdio.h>
#include <string.h>

extern const uint8_t _binary_index_html_start[];

void app_main(void) {
    while(1){
        printf("%s\n", _binary_index_html_start);
        printf("aswd\n");
    }
}