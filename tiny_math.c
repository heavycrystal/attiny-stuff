#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <math.h>

#define ALPHA 1.0e-6

uint8_t w[14 * 14];
uint8_t x[14 * 14];

double sigmoid(double input)
{
    return 1.0 / (1.0 + exp(-input));
}

double loss_calc(bool true_value, double computed_value)
{
    return (true_value) ? -log(computed_value) : -log(1 - computed_value);
}

double matrix_multiply(uint8_t* w, uint8_t* x)
{

}
