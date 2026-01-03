#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>


// file for payments
#define FILENAME "payments.dat"

// to store payments
struct Payment {
    char name[100];
    float fees;
    char info[200];
};

void readLine(char *buffer, int size) {
    fgets(buffer, size, stdin);
    buffer[strcspn(buffer, "\n")] = 0;
}

void answerYesOrNo(char *choice) {
    while (true) {
        readLine(choice, 10);

        for (int i = 0; choice[i]; i++) {
            choice[i] = tolower(choice[i]);
        }

        if (strcmp(choice, "yes") == 0 || strcmp(choice, "no") == 0) {
            return;
        }

        printf("Invalid answer! Type 'yes' or 'no' in lowercase ");
    }
}


float getPositiveFloat() {
    char buffer[50];
    float value;
    char *endptr; // Pointer to where the conversion stops

    while (true) {
        readLine(buffer, sizeof(buffer));

        errno = 0;

        // from string to float
        value = strtof(buffer, &endptr);

        // 1. Check for conversion errors (errno != 0 means overflow/underflow)
        // 2. Check if the conversion read anything (endptr != buffer)
        // 3. Check if there are any non-whitespace, unconverted characters left (*endptr == '\0' or is only whitespace)
        // Since readLine removes the newline, we just check if endptr points to the null terminator.
        
        bool conversion_success = (endptr != buffer && *endptr == '\0' && errno == 0);

        if (conversion_success) {
            if (value > 0.0f) {
                return value;
            }
        }

        printf("Invalid number! Enter a POSITIVE fee amount: ");
    }
}

bool nameExists(struct Payment *payments, int paymentCount, const char *name) {
    for (int i = 0; i < paymentCount; i++) {
        if (strcmp(payments[i].name, name) == 0) {
            // name already exist
            return true;
        }
    }

    return false;
}

void saveToFile(struct Payment *payments, int count) {
    FILE *file = fopen(FILENAME, "wb");
    // checks if file is missing
    if (file == NULL) {
        printf("Error: Could not open file for saving.\n");
        return;
    }
    // writes the count of the payments
    fwrite(&count, sizeof(int), 1, file);
    // writes the data of the payments
    fwrite(payments, sizeof(struct Payment), count, file);
    fclose(file);
    printf("Data saved successfully to %s\n", FILENAME);
}

struct Payment* loadFromFile(int *count) {
    FILE *file = fopen(FILENAME, "rb");
    // checks if file is missing
    if (file == NULL) {
        *count = 0;
        return NULL;
    }

    fread(count, sizeof(int), 1, file);
    struct Payment *payments = malloc((*count) * sizeof(struct Payment));
    
    if (payments != NULL) {
        fread(payments, sizeof(struct Payment), *count, file);
    }
    
    fclose(file);
    return payments;
}

bool isBlank(const char *str) {
    // if string is empty
    if (str[0] == '\0') 
        return true;

    for (int i = 0; str[i] != '\0'; i++) {
        if (!isspace((unsigned char)str[i])) {
            return false;
        }
    }
    return true;
}


int main() {
    int paymentCount = 0;
    struct Payment *payments = loadFromFile(&paymentCount);
    char choice[10];
    char input[200];

    printf("Welcome to School Fee Management System!\n");
    printf("Current payments loaded: %d\n", paymentCount);


    // Asks if user wants to add new payment
    // yes -> adds payment (name, fee, info)
    // no -> asks if user want to see the list of the payments
    printf("Do you want to add a new payment? (yes/no): ");
    answerYesOrNo(choice);

    if (strcmp(choice, "yes") == 0) {

        while (true) {
            struct Payment newPayment;

            printf("\nEnter name of the payment(or type 'stop' to finish): ");
            readLine(newPayment.name, sizeof(newPayment.name));

            if (strcmp(newPayment.name, "stop") == 0) {
                break;
            }

            if(isBlank(newPayment.name)) {
                printf("Name cannot be empty or whitespace!\n");
                continue;
            }

            if (nameExists(payments, paymentCount, newPayment.name)) {
                printf("A payment with that name already exists. Please enter diffrent name.\n");
                continue;
            }

            printf("Enter fee amount: ");
            newPayment.fees = getPositiveFloat();

            printf("Enter additional info: ");
            readLine(newPayment.info, sizeof(newPayment.info));

            // if this is missing -> throws segmentation fault
            payments = realloc(payments, (paymentCount + 1) * sizeof(struct Payment));
            if (payments == NULL) {
                printf("Memory allocation failed!\n");
                return 1;
            }

            // payment is put in structure
            payments[paymentCount] = newPayment;
            paymentCount++;
        }
        saveToFile(payments, paymentCount);
    }

    if (paymentCount > 0) {

        printf("\nDo you want to see the list of payments? (yes/no): ");
        answerYesOrNo(choice);

        if (strcmp(choice, "yes") == 0) {
            // if there are no payments -> printf
            // else -> iterate payments (structure)
            if (paymentCount == 0) {
                printf("No payments have been found!\n");
            } else {
                bool done = false;

                // shows the list of payments (only names and their count)
                while (!done) {
                    printf("\nList of payments:\n");
                    for (int i = 0; i < paymentCount; i++) {
                        printf("%d. %s\n", i + 1, payments[i].name);
                    }

                    printf("\nEnter a name to view details of a payment: ");
                    readLine(input, sizeof(input));

                    // give data for the chosen payment
                    bool found = false;
                    for (int i = 0; i < paymentCount; i++) {
                        if (strcmp(payments[i].name, input) == 0) {
                            printf("\nPayment Details:\n");
                            printf("Name : %s\n", payments[i].name);
                            printf("Fee : %.2f\n", payments[i].fees);
                            printf("Info : %s\n", payments[i].info);
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        printf("No payment found with that name.\n");
                    }

                    printf("\nDo you want to see another payment? (yes/no): ");
                    answerYesOrNo(choice);

                    if (strcmp(choice, "no") == 0) {
                        done = true;
                    }
                }
            }
        }
    } else {
        printf("\nNo payments available.\n");
    }

    free(payments);

    printf("\nSee you soon!\n");
    return 0;
}
