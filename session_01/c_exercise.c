#include <stdio.h>
#include <string.h>


int main() {

    // 1)

    typedef struct route {
        int routeID;
        char descrp[26];
    } Route;

    Route route1;
    Route longroutes[10];
    Route* routePtr;

    printf("Type in Route ID! \n");
    unsigned int routeID_in;
    scanf("%d", &routeID_in);
    route1.routeID = routeID_in;

    printf("Type in route description! \n");
    char description_in[26];
    scanf("%25s", description_in);
    memcpy(route1.descrp, description_in, sizeof route1.descrp);
    /*route1.descrp = description_in;*/

    longroutes[2].routeID = route1.routeID;
    memcpy(longroutes[2].descrp, route1.descrp, sizeof longroutes[2].descrp);
    /*longroutes[2].descrp = route1.descrp;*/

    routePtr = &longroutes[2];

    printf("Route ID: %d \n", (*routePtr).routeID);
    printf("Route Description: %s \n", (*routePtr).descrp);
    

    // 2)
    
    double taxrate = 7.3, discountrate;
    char buyer[100], seller[100];

    double* tmpPtr;
    tmpPtr = &taxrate;
    printf("Variable pointed to: %f \n", *tmpPtr);
    discountrate = *tmpPtr;
    printf("Discount rate: %f \n", discountrate);
    printf("Address of taxrate variable: %p \n", &taxrate);
    printf("Address stored in tmpPtr: %p \n", tmpPtr);
    // YES!
    strncpy(buyer, "Buyer Name", sizeof buyer);
    memcpy(seller, buyer, sizeof seller);
    int compare_result = strcmp(buyer, seller);
    printf("Compare result: %d \n", compare_result);
    strcat(buyer, seller);
    printf("Appended: %s \n", buyer);


    

}
