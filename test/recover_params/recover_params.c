//
// Created by pierfied on 9/12/17.
//

#include <stdlib.h>
#include <time.h>
#include "recover_params.h"
#include "include/hmc.h"
#include "include/normal.h"

Hamiltonian RP_logp(double *params, void *arg_ptr){
    RPLikelihoodArgs *args = (RPLikelihoodArgs *)arg_ptr;

    int n_data = args->n_data;
    double *y = args->y;
    double *x = args->x;
    double *err = args->err;

    double m = params[0];
    double b = params[1];
    double y_model;
    double log_likelihood = 0;
    double *grad = malloc(sizeof(double) * 2);
    grad[0] = 0;
    grad[1] = 0;
    for(int i = 0; i < n_data; i++){
        y_model = m * x[i] + b;

        log_likelihood += (y[i] - y_model)*(y[i] - y_model)/(err[i] * err[i]);

        grad[0] += (y[i] - y_model)*x[i]/(err[i] * err[i]);
        grad[1] += (y[i] - y_model)/(err[i] * err[i]);
    }
    log_likelihood *= -0.5;

    Hamiltonian H;
    H.log_likelihood = log_likelihood;
    H.grad = grad;

    return H;
}

RPSampleResults RP_test(int n_data, int num_samples, int num_steps,
                   int num_burn, double epsilon){
    srandom(time(NULL));

    RPLikelihoodArgs args;
    args.n_data = n_data;

    double m_true = 2;
    double b_true = -1;
    double err_mean = 0.75;
    double err_sigma = 0.25;

    int num_params = 2;
    double params0[num_params];
    double *y_data = malloc(sizeof(double) * n_data);
    double *x = malloc(sizeof(double) * n_data);
    double *err = malloc(sizeof(double) * n_data);
    for(int i = 0; i < n_data; i++){
        x[i] = i;
        err[i] = err_mean + err_sigma * normal();
        y_data[i] = m_true*x[i] + b_true + err[i]*normal();
    }
    args.y = y_data;
    args.x = x;
    args.err = err;

    params0[0] = normal();
    params0[1] = normal();

    HMCArgs hmc_args;
    hmc_args.log_likelihood = RP_logp;
    hmc_args.likelihood_args = (void *)&args;
    hmc_args.num_samples = num_samples;
    hmc_args.num_params = num_params;
    hmc_args.num_steps = num_steps;
    hmc_args.num_burn = num_burn;
    hmc_args.epsilon = epsilon;
    hmc_args.x0 = params0;

    SampleChain chain = hmc(hmc_args);
    RPSampleResults results;
    results.chain = chain;
    results.m_true = m_true;
    results.b_true = b_true;
    results.y = y_data;
    results.x = x;
    results.err = err;

    return results;
}