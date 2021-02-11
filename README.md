# K-V-Workload-Generator

This repository maintains a key-value workload generator that supports different distributions (uniform, normal and zipfian) with customized parameters for inserts, updates and point queries. Other parameters such as zero-result point lookup ratio, range lookup selectivty can be also specified. Run `make` and you will have an executable file 'load_gen' which receives the following parameters:

  ./load_gen {OPTIONS}

    workload_gen_parser

  OPTIONS:

      This group is all exclusive:
        -I[I], --insert=[I]               Number of inserts [def: 1]
        -U[U], --update=[U]               Number of updates [def: 0]
        -D[D], --point_delete=[D]         Number of point deletes [def: 0]
        -R[R], --range_delete=[R]         Number of range deletes [def: 0]
        -y[y],
        --range_delete_selectivity=[y]    Range delete selectivity [def: 0]
        -Q[Q], --point_query=[Q]          Number of point queries [def: 0]
        -S[S], --range_query=[S]          Number of range queries [def: 0]
        -Y[Y],
        --range_query_selectivity=[Y]     Range query selectivity [def: 0]
        -z[z],
        --zero_result_point_delete_proportion=[z]
                                          Proportion of zero-result point
                                          deletes [def: 0]
        -Z[Z],
        --zero_result_point_lookup_proportion=[Z]
                                          Proportion of zero-result point
                                          lookups [def: 0]
        --UZ=[UZ],
        --unique_zero_result_point_lookup_proportion=[UZ]
                                          Proportion of maximum unique
                                          zero-result point lookups [def: 0.5]
        --UE=[UE],
        --maximum_unique_existing_point_lookup_proportion=[UE]
                                          Proportion of maximum unique exising
                                          point lookups [def: 0.5]
        -E[E], --entry_size=[E]           Entry size (in bytes) [def: 8]
        -L[L], --lambda=[L]               lambda = key_size / (key_size +
                                          value_size) [def: 0.5]
        --PL, --preloading                preload from workload.txt
        --OP=[OP], --output-path=[OP]     output path [def: 0]
        --ID=[ID],
        --insert_distribution=[ID]        Insert Distribution [0: uniform,
                                          1:normal, 2:beta, 3:zipf, def: 0]
        --ID_NMP=[ID_Norm_Mean_Percentile],
        --insert_distribution_norm_mean_percentile=[ID_Norm_Mean_Percentile]
                                          , def: 0.5]
        --ID_NDEV=[ID_Norm_Stddev],
        --insert_distribution_norm_standard_deviation=[ID_Norm_Stddev]
                                          , def: 1]
        --ID_BALPHA=[ID_Beta_Alpha],
        --insert_distribution_beta_alpha=[ID_Beta_Alpha]
                                          , def: 1.0]
        --ID_BBETA=[ID_Beta_Beta],
        --insert_distribution_beta_beta=[ID_Beta_Beta]
                                          , def: 1.0]
        --ID_ZALPHA=[ID_Zipf_Alpha],
        --insert_distribution_zipf_alpha=[ID_Zipf_Alpha]
                                          , def: 1.0]
        --UD=[UD],
        --update_distribution=[UD]        Update Distribution [0: uniform,
                                          1:normal, 2:beta, 3:zipf, def: 0]
        --UD_NMP=[UD_Norm_Mean_Percentile],
        --update_distribution_norm_mean_percentile=[UD_Norm_Mean_Percentile]
                                          , def: 0.5]
        --UD_NDEV=[UD_Norm_Stddev],
        --update_distribution_norm_standard_deviation=[UD_Norm_Stddev]
                                          , def: 1]
        --UD_BALPHA=[UD_Beta_Alpha],
        --update_distribution_beta_alpha=[UD_Beta_Alpha]
                                          , def: 1.0]
        --UD_BBETA=[UD_Beta_Beta],
        --update_distribution_beta_beta=[UD_Beta_Beta]
                                          , def: 1.0]
        --UD_ZALPHA=[UD_Zipf_Alpha],
        --update_distribution_zipf_alpha=[UD_Zipf_Alpha]
                                          , def: 1.0]
        --ED=[ED],
        --existing_point_lookup_distribution=[ED]
                                          Existing Point Lookup Distribution [0:
                                          uniform, 1:normal, 2:beta, 3:zipf,
                                          def: 0]
        --ED_NMP=[ED_Norm_Mean_Percentile],
        --existing_point_lookup_distribution_norm_mean_percentile=[ED_Norm_Mean_Percentile]
                                          , def: 0.5]
        --ED_NDEV=[ED_Norm_Stddev],
        --existing_point_lookup_distribution_norm_standard_deviation=[ED_Norm_Stddev]
                                          , def: 1]
        --ED_BALPHA=[ED_Beta_Alpha],
        --existing_point_lookup_distribution_beta_alpha=[ED_Beta_Alpha]
                                          , def: 1.0]
        --ED_BBETA=[ED_Beta_Beta],
        --existing_point_lookup_distribution_beta_beta=[ED_Beta_Beta]
                                          , def: 1.0]
        --ED_ZALPHA=[ED_Zipf_Alpha],
        --existing_point_lookup_distribution_zipf_alpha=[ED_Zipf_Alpha]
                                          , def: 1.0]
        --ZD=[ZD],
        --non_existing_point_lookup_distribution=[ZD]
                                          Zero-result Point Lookup Distribution
                                          [0: uniform, 1:normal, 2:beta, 3:zipf,
                                          def: 0]
        --ZD_NMP=[ZD_Norm_Mean_Percentile],
        --non_existing_point_lookup_distribution_norm_mean_percentile=[ZD_Norm_Mean_Percentile]
                                          , def: 0.5]
        --ZD_NDEV=[ZD_Norm_Stddev],
        --non_existing_point_lookup_distribution_norm_standard_deviation=[ZD_Norm_Stddev]
                                          , def: 1]
        --ZD_BALPHA=[ZD_Beta_Alpha],
        --non_existing_point_lookup_distribution_beta_alpha=[ZD_Beta_Alpha]
                                          , def: 1.0]
        --ZD_BBETA=[ZD_Beta_Beta],
        --non_existing_point_lookup_distribution_beta_beta=[ZD_Beta_Beta]
                                          , def: 1.0]
        --ZD_ZALPHA=[ZD_Zipf_Alpha],
        --non_existing_point_lookup_distribution_zipf_alpha=[ZD_Zipf_Alpha]
                                          , def: 1.0]
