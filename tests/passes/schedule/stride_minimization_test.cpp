#include "sdfg/passes/schedule/stride_minimization.h"

#include <gtest/gtest.h>

#include "fixtures/polybench.h"
#include "fixtures/rodinia.h"
#include "sdfg/passes/schedule/perfect_loop_distribution.h"

using namespace sdfg;

TEST(StrideMinimizationTest, is_admissible) {
    std::vector<std::string> current_1 = {"i", "j", "k"};
    std::vector<std::string> target_1 = {"j", "i", "k"};
    std::unordered_set<std::string> allowed_swaps_1 = {"i_j"};
    EXPECT_TRUE(passes::StrideMinimization::is_admissible(current_1, target_1, allowed_swaps_1));

    std::unordered_set<std::string> allowed_swaps_2 = {"j_i"};
    EXPECT_FALSE(passes::StrideMinimization::is_admissible(current_1, target_1, allowed_swaps_2));

    std::unordered_set<std::string> allowed_swaps_3 = {"j_k"};
    EXPECT_FALSE(passes::StrideMinimization::is_admissible(current_1, target_1, allowed_swaps_3));

    std::vector<std::string> current_2 = {"i", "k", "j"};
    std::vector<std::string> target_2 = {"k", "i", "j"};
    std::unordered_set<std::string> allowed_swaps_4 = {"i_k", "k_j"};
    EXPECT_TRUE(passes::StrideMinimization::is_admissible(current_2, target_2, allowed_swaps_4));

    std::vector<std::string> target_3 = {"k", "j", "i"};
    EXPECT_FALSE(passes::StrideMinimization::is_admissible(current_2, target_3, allowed_swaps_4));
}

TEST(StrideMinimizationTest, Polybench_correlation) {
    // Build SDFG
    auto init_sdfg = correlation();
    auto schedule = std::make_unique<Schedule>(init_sdfg);

    // Pass
    passes::PerfectLoopDistribution pass;
    bool applies;
    do {
        applies = false;
        applies = pass.run(*schedule);
    } while (applies);

    passes::StrideMinimization stride_minimization_pass;
    do {
        applies = false;
        applies = stride_minimization_pass.run(*schedule);
    } while (applies);

    auto& sdfg = schedule->builder().subject();

    // Check
    /***
    for (i = 0; i < _PB_M-1; i++)
        C[i][i] = SCALAR_VAL(1.0);

    for (i = 0; i < _PB_M-1; i++)
        for (j = i+1; j < _PB_M; j++)
            C[i][j] = SCALAR_VAL(0.0);

    for (i = 0; i < _PB_M-1; i++)
        for (k = 0; k < _PB_N; k++)
            for (j = i+1; j < _PB_M; j++)
                C[i][j] += (D[k][i] * D[k][j]);

    for (i = 0; i < _PB_M-1; i++)
        for (j = i+1; j < _PB_M; j++)
            C[j][i] = C[i][j];
    ***/

    {
        auto& root = sdfg.root();
        EXPECT_EQ(root.size(), 4);

        auto loop_i_1 = dynamic_cast<structured_control_flow::For*>(&root.at(0).first);
        EXPECT_TRUE(loop_i_1 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_i_1->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(
            *loop_i_1->condition(),
            *symbolic::Lt(loop_i_1->indvar(),
                          symbolic::sub(symbolic::symbol("M"), symbolic::integer(1)))));
        EXPECT_TRUE(SymEngine::eq(*loop_i_1->update(),
                                  *symbolic::add(loop_i_1->indvar(), symbolic::integer(1))));
        EXPECT_EQ(loop_i_1->root().size(), 1);

        auto loop_i_2 = dynamic_cast<structured_control_flow::For*>(&root.at(1).first);
        EXPECT_TRUE(loop_i_2 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_i_2->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(
            *loop_i_2->condition(),
            *symbolic::Lt(loop_i_2->indvar(),
                          symbolic::sub(symbolic::symbol("M"), symbolic::integer(1)))));
        EXPECT_TRUE(SymEngine::eq(*loop_i_2->update(),
                                  *symbolic::add(loop_i_2->indvar(), symbolic::integer(1))));
        EXPECT_EQ(loop_i_2->root().size(), 1);
        auto loop_j_2 = dynamic_cast<structured_control_flow::For*>(&loop_i_2->root().at(0).first);
        EXPECT_TRUE(loop_j_2 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_j_2->init(),
                                  *symbolic::add(loop_i_2->indvar(), symbolic::integer(1))));
        EXPECT_TRUE(SymEngine::eq(*loop_j_2->condition(),
                                  *symbolic::Lt(loop_j_2->indvar(), symbolic::symbol("M"))));
        EXPECT_TRUE(SymEngine::eq(*loop_j_2->update(),
                                  *symbolic::add(loop_j_2->indvar(), symbolic::integer(1))));

        auto loop_i_3 = dynamic_cast<structured_control_flow::For*>(&root.at(2).first);
        EXPECT_TRUE(loop_i_3 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_i_3->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(
            *loop_i_3->condition(),
            *symbolic::Lt(loop_i_3->indvar(),
                          symbolic::sub(symbolic::symbol("M"), symbolic::integer(1)))));
        EXPECT_TRUE(SymEngine::eq(*loop_i_3->update(),
                                  *symbolic::add(loop_i_3->indvar(), symbolic::integer(1))));
        EXPECT_EQ(loop_i_3->root().size(), 1);
        auto loop_k_3 = dynamic_cast<structured_control_flow::For*>(&loop_i_3->root().at(0).first);
        EXPECT_TRUE(loop_k_3 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_k_3->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_k_3->condition(),
                                  *symbolic::Lt(loop_k_3->indvar(), symbolic::symbol("N"))));
        EXPECT_TRUE(SymEngine::eq(*loop_k_3->update(),
                                  *symbolic::add(loop_k_3->indvar(), symbolic::integer(1))));
        auto loop_j_3 = dynamic_cast<structured_control_flow::For*>(&loop_k_3->root().at(0).first);
        EXPECT_TRUE(loop_j_3 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_j_3->init(),
                                  *symbolic::add(loop_i_3->indvar(), symbolic::integer(1))));
        EXPECT_TRUE(SymEngine::eq(*loop_j_3->condition(),
                                  *symbolic::Lt(loop_j_3->indvar(), symbolic::symbol("M"))));
        EXPECT_TRUE(SymEngine::eq(*loop_j_3->update(),
                                  *symbolic::add(loop_j_3->indvar(), symbolic::integer(1))));
        EXPECT_EQ(loop_j_3->root().size(), 1);

        auto loop_i_4 = dynamic_cast<structured_control_flow::For*>(&root.at(3).first);
        EXPECT_TRUE(loop_i_4 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_i_4->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(
            *loop_i_4->condition(),
            *symbolic::Lt(loop_i_4->indvar(),
                          symbolic::sub(symbolic::symbol("M"), symbolic::integer(1)))));
        EXPECT_TRUE(SymEngine::eq(*loop_i_4->update(),
                                  *symbolic::add(loop_i_4->indvar(), symbolic::integer(1))));
        EXPECT_EQ(loop_i_4->root().size(), 1);
        auto loop_j_4 = dynamic_cast<structured_control_flow::For*>(&loop_i_4->root().at(0).first);
        EXPECT_TRUE(loop_j_4 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_j_4->init(),
                                  *symbolic::add(loop_i_4->indvar(), symbolic::integer(1))));
        EXPECT_TRUE(SymEngine::eq(*loop_j_4->condition(),
                                  *symbolic::Lt(loop_j_4->indvar(), symbolic::symbol("M"))));
        EXPECT_TRUE(SymEngine::eq(*loop_j_4->update(),
                                  *symbolic::add(loop_j_4->indvar(), symbolic::integer(1))));
    }
}

TEST(StrideMinimizationTest, Polybench_covariance) {
    // Build SDFG
    auto init_sdfg = covariance();
    auto schedule = std::make_unique<Schedule>(init_sdfg);

    // Pass
    passes::PerfectLoopDistribution pass;
    bool applies;
    do {
        applies = false;
        applies = pass.run(*schedule);
    } while (applies);

    passes::StrideMinimization stride_minimization_pass;
    do {
        applies = false;
        applies = stride_minimization_pass.run(*schedule);
    } while (applies);

    auto& sdfg = schedule->builder().subject();

    // Check
    /***
    for (i = 0; i < _PB_M; i++)
        for (j = i; j < _PB_M; j++)
            C[i][j] = SCALAR_VAL(0.0);

    for (i = 0; i < _PB_M; i++)
        for (k = 0; k < _PB_N; k++)
            for (j = i; j < _PB_M; j++)
                C[i][j] += (D[k][i] * D[k][j]);

    for (i = 0; i < _PB_M; i++)
        for (j = i; j < _PB_M; j++)
            C[j][i] = C[i][j];
    ***/

    {
        auto& root = sdfg.root();
        EXPECT_EQ(root.size(), 3);

        auto loop_i_2 = dynamic_cast<structured_control_flow::For*>(&root.at(0).first);
        EXPECT_TRUE(loop_i_2 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_i_2->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_i_2->condition(),
                                  *symbolic::Lt(loop_i_2->indvar(), symbolic::symbol("M"))));
        EXPECT_TRUE(SymEngine::eq(*loop_i_2->update(),
                                  *symbolic::add(loop_i_2->indvar(), symbolic::integer(1))));
        EXPECT_EQ(loop_i_2->root().size(), 1);
        auto loop_j_2 = dynamic_cast<structured_control_flow::For*>(&loop_i_2->root().at(0).first);
        EXPECT_TRUE(loop_j_2 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_j_2->init(), *loop_i_2->indvar()));
        EXPECT_TRUE(SymEngine::eq(*loop_j_2->condition(),
                                  *symbolic::Lt(loop_j_2->indvar(), symbolic::symbol("M"))));
        EXPECT_TRUE(SymEngine::eq(*loop_j_2->update(),
                                  *symbolic::add(loop_j_2->indvar(), symbolic::integer(1))));

        auto loop_i_3 = dynamic_cast<structured_control_flow::For*>(&root.at(1).first);
        EXPECT_TRUE(loop_i_3 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_i_3->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_i_3->condition(),
                                  *symbolic::Lt(loop_i_3->indvar(), symbolic::symbol("M"))));
        EXPECT_TRUE(SymEngine::eq(*loop_i_3->update(),
                                  *symbolic::add(loop_i_3->indvar(), symbolic::integer(1))));
        EXPECT_EQ(loop_i_3->root().size(), 1);
        auto loop_k_3 = dynamic_cast<structured_control_flow::For*>(&loop_i_3->root().at(0).first);
        EXPECT_TRUE(loop_k_3 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_k_3->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_k_3->condition(),
                                  *symbolic::Lt(loop_k_3->indvar(), symbolic::symbol("N"))));
        EXPECT_TRUE(SymEngine::eq(*loop_k_3->update(),
                                  *symbolic::add(loop_k_3->indvar(), symbolic::integer(1))));
        auto loop_j_3 = dynamic_cast<structured_control_flow::For*>(&loop_k_3->root().at(0).first);
        EXPECT_TRUE(loop_j_3 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_j_3->init(), *loop_i_3->indvar()));
        EXPECT_TRUE(SymEngine::eq(*loop_j_3->condition(),
                                  *symbolic::Lt(loop_j_3->indvar(), symbolic::symbol("M"))));
        EXPECT_TRUE(SymEngine::eq(*loop_j_3->update(),
                                  *symbolic::add(loop_j_3->indvar(), symbolic::integer(1))));
        EXPECT_EQ(loop_j_3->root().size(), 1);

        auto loop_i_4 = dynamic_cast<structured_control_flow::For*>(&root.at(2).first);
        EXPECT_TRUE(loop_i_4 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_i_4->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_i_4->condition(),
                                  *symbolic::Lt(loop_i_4->indvar(), symbolic::symbol("M"))));
        EXPECT_TRUE(SymEngine::eq(*loop_i_4->update(),
                                  *symbolic::add(loop_i_4->indvar(), symbolic::integer(1))));
        EXPECT_EQ(loop_i_4->root().size(), 1);
        auto loop_j_4 = dynamic_cast<structured_control_flow::For*>(&loop_i_4->root().at(0).first);
        EXPECT_TRUE(loop_j_4 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_j_4->init(), *loop_i_4->indvar()));
        EXPECT_TRUE(SymEngine::eq(*loop_j_4->condition(),
                                  *symbolic::Lt(loop_j_4->indvar(), symbolic::symbol("M"))));
        EXPECT_TRUE(SymEngine::eq(*loop_j_4->update(),
                                  *symbolic::add(loop_j_4->indvar(), symbolic::integer(1))));
    }
}

TEST(StrideMinimizationTest, Polybench_gemm) {
    // Build SDFG
    auto init_sdfg = gemm();
    auto schedule = std::make_unique<Schedule>(init_sdfg);

    // Pass
    passes::PerfectLoopDistribution pass;
    bool applies;
    do {
        applies = false;
        applies = pass.run(*schedule);
    } while (applies);

    passes::StrideMinimization stride_minimization_pass;
    do {
        applies = false;
        applies = stride_minimization_pass.run(*schedule);
    } while (applies);

    auto& sdfg = schedule->builder().subject();

    // Check
    /***
     for (i = 0; i < _PB_N; i++) {
        for (j = 0; j < _PB_N; j++)
            C[i][j] *= beta;
    }
    for (i = 0; i < _PB_N; i++) {
        for (k = 0; k < _PB_N; k++) {
            for (j = 0; j < _PB_N; j++)
                C[i][j] += alpha * A[i][k] * B[k][j];
        }
    }
    ***/

    {
        auto& root = sdfg.root();
        EXPECT_EQ(root.size(), 2);

        auto loop_i_1 = dynamic_cast<structured_control_flow::For*>(&root.at(0).first);
        EXPECT_TRUE(loop_i_1 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_i_1->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_i_1->condition(),
                                  *symbolic::Lt(loop_i_1->indvar(), symbolic::symbol("N"))));
        EXPECT_TRUE(SymEngine::eq(*loop_i_1->update(),
                                  *symbolic::add(loop_i_1->indvar(), symbolic::integer(1))));
        EXPECT_EQ(loop_i_1->root().size(), 1);
        auto loop_j_1 = dynamic_cast<structured_control_flow::For*>(&loop_i_1->root().at(0).first);
        EXPECT_TRUE(loop_j_1 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_j_1->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_j_1->condition(),
                                  *symbolic::Lt(loop_j_1->indvar(), symbolic::symbol("M"))));
        EXPECT_TRUE(SymEngine::eq(*loop_j_1->update(),
                                  *symbolic::add(loop_j_1->indvar(), symbolic::integer(1))));

        auto loop_i_2 = dynamic_cast<structured_control_flow::For*>(&root.at(1).first);
        EXPECT_TRUE(loop_i_2 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_i_2->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_i_2->condition(),
                                  *symbolic::Lt(loop_i_2->indvar(), symbolic::symbol("N"))));
        EXPECT_TRUE(SymEngine::eq(*loop_i_2->update(),
                                  *symbolic::add(loop_i_2->indvar(), symbolic::integer(1))));
        EXPECT_EQ(loop_i_2->root().size(), 1);
        auto loop_k_2 = dynamic_cast<structured_control_flow::For*>(&loop_i_2->root().at(0).first);
        EXPECT_TRUE(loop_k_2 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_k_2->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_k_2->condition(),
                                  *symbolic::Lt(loop_k_2->indvar(), symbolic::symbol("K"))));
        EXPECT_TRUE(SymEngine::eq(*loop_k_2->update(),
                                  *symbolic::add(loop_k_2->indvar(), symbolic::integer(1))));
        EXPECT_EQ(loop_k_2->root().size(), 1);
        auto loop_j_2 = dynamic_cast<structured_control_flow::For*>(&loop_k_2->root().at(0).first);
        EXPECT_TRUE(loop_j_2 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_j_2->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_j_2->condition(),
                                  *symbolic::Lt(loop_j_2->indvar(), symbolic::symbol("M"))));
        EXPECT_TRUE(SymEngine::eq(*loop_j_2->update(),
                                  *symbolic::add(loop_j_2->indvar(), symbolic::integer(1))));
    }
}

TEST(StrideMinimizationTest, Polybench_symm) {
    // Build SDFG
    auto init_sdfg = symm();
    auto schedule = std::make_unique<Schedule>(init_sdfg);

    // Pass
    passes::PerfectLoopDistribution pass;
    bool applies;
    do {
        applies = false;
        applies = pass.run(*schedule);
    } while (applies);

    passes::StrideMinimization stride_minimization_pass;
    do {
        applies = false;
        applies = stride_minimization_pass.run(*schedule);
    } while (applies);

    auto& sdfg = schedule->builder().subject();

    // Check
    /***
    for (i = 0; i < _PB_M; i++) {
        for (j = 0; j < _PB_N; j++ ) {
            temp2[i][j] = 0;
        }
    }
    for (i = 0; i < _PB_M; i++) {
        for (k = 0; k < i; k++) {
            for (j = 0; j < _PB_N; j++ ) {
                C[k][j] += B[i][j] * A[i][k];
                temp2[i][j] += B[k][j] * A[i][k];
            }
        }
        for (j = 0; j < _PB_N; j++ ) {
            C[i][j] = C[i][j] + temp2[i][j];
        }
    }
    ***/

    {
        auto& root = sdfg.root();
        EXPECT_EQ(root.size(), 2);

        types::Scalar desc_element(types::PrimitiveType::Double);
        types::Array tmp_1d(desc_element, symbolic::integer(12));
        types::Array tmp_2d(tmp_1d, symbolic::integer(16));
        EXPECT_EQ(sdfg.type("tmp"), tmp_2d);

        auto loop_i_1 = dynamic_cast<structured_control_flow::For*>(&root.at(0).first);
        EXPECT_TRUE(loop_i_1 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_i_1->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_i_1->condition(),
                                  *symbolic::Lt(loop_i_1->indvar(), symbolic::integer(16))));
        EXPECT_TRUE(SymEngine::eq(*loop_i_1->update(),
                                  *symbolic::add(loop_i_1->indvar(), symbolic::integer(1))));

        auto& body_i_1 = loop_i_1->root();
        EXPECT_EQ(body_i_1.size(), 1);
        {
            auto loop_j_1 = dynamic_cast<structured_control_flow::For*>(&body_i_1.at(0).first);
            EXPECT_TRUE(loop_j_1 != nullptr);
            EXPECT_TRUE(SymEngine::eq(*loop_j_1->init(), *symbolic::integer(0)));
            EXPECT_TRUE(SymEngine::eq(*loop_j_1->condition(),
                                      *symbolic::Lt(loop_j_1->indvar(), symbolic::integer(12))));
            EXPECT_TRUE(SymEngine::eq(*loop_j_1->update(),
                                      *symbolic::add(loop_j_1->indvar(), symbolic::integer(1))));
        }

        auto loop_i_2 = dynamic_cast<structured_control_flow::For*>(&root.at(1).first);
        EXPECT_TRUE(loop_i_2 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_i_2->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_i_2->condition(),
                                  *symbolic::Lt(loop_i_2->indvar(), symbolic::integer(16))));
        EXPECT_TRUE(SymEngine::eq(*loop_i_2->update(),
                                  *symbolic::add(loop_i_2->indvar(), symbolic::integer(1))));

        auto& body_i_2 = loop_i_2->root();
        EXPECT_EQ(body_i_2.size(), 2);
        {
            auto loop_k =
                dynamic_cast<structured_control_flow::For*>(&loop_i_2->root().at(0).first);
            EXPECT_TRUE(loop_k != nullptr);
            EXPECT_TRUE(SymEngine::eq(*loop_k->init(), *symbolic::integer(0)));
            EXPECT_TRUE(SymEngine::eq(*loop_k->condition(),
                                      *symbolic::Lt(loop_k->indvar(), loop_i_2->indvar())));
            EXPECT_TRUE(SymEngine::eq(*loop_k->update(),
                                      *symbolic::add(loop_k->indvar(), symbolic::integer(1))));
            {
                auto loop_j_2 =
                    dynamic_cast<structured_control_flow::For*>(&loop_k->root().at(0).first);
                EXPECT_TRUE(loop_j_2 != nullptr);
                EXPECT_TRUE(SymEngine::eq(*loop_j_2->init(), *symbolic::integer(0)));
                EXPECT_TRUE(
                    SymEngine::eq(*loop_j_2->condition(),
                                  *symbolic::Lt(loop_j_2->indvar(), symbolic::integer(12))));
                EXPECT_TRUE(SymEngine::eq(
                    *loop_j_2->update(), *symbolic::add(loop_j_2->indvar(), symbolic::integer(1))));
            }

            auto loop_j_3 = dynamic_cast<structured_control_flow::For*>(&body_i_2.at(1).first);
            EXPECT_TRUE(loop_j_3 != nullptr);
            EXPECT_TRUE(SymEngine::eq(*loop_j_3->init(), *symbolic::integer(0)));
            EXPECT_TRUE(SymEngine::eq(*loop_j_3->condition(),
                                      *symbolic::Lt(loop_j_3->indvar(), symbolic::integer(12))));
            EXPECT_TRUE(SymEngine::eq(*loop_j_3->update(),
                                      *symbolic::add(loop_j_3->indvar(), symbolic::integer(1))));
        }
    }
}

TEST(StrideMinimizationTest, Polybench_gemver) {
    // Build SDFG
    auto init_sdfg = gemver();
    auto schedule = std::make_unique<Schedule>(init_sdfg);

    // Pass
    passes::PerfectLoopDistribution pass;
    bool applies;
    do {
        applies = false;
        applies = pass.run(*schedule);
    } while (applies);

    passes::StrideMinimization stride_minimization_pass;
    do {
        applies = false;
        applies = stride_minimization_pass.run(*schedule);
    } while (applies);

    auto& sdfg = schedule->builder().subject();

    // Check
    /***
    for (i = 0; i < _PB_N; i++)
        for (j = 0; j < _PB_N; j++)
            A[i][j] = A[i][j] + u1[i] * v1[j] + u2[i] * v2[j];

    for (j = 0; j < _PB_N; j++)
        for (i = 0; i < _PB_N; i++)
            x[i] = x[i] + A[j][i] * y[j];

    for (i = 0; i < _PB_N; i++)
        x[i] = x[i] + z[i];

    for (i = 0; i < _PB_N; i++)
        for (j = 0; j < _PB_N; j++)
            w[i] = w[i] + A[i][j] * x[j];
    */

    {
        auto& root = sdfg.root();
        EXPECT_EQ(root.size(), 4);

        auto loop_i_1 = dynamic_cast<structured_control_flow::For*>(&root.at(0).first);
        EXPECT_TRUE(loop_i_1 != nullptr);
        EXPECT_EQ(loop_i_1->indvar()->get_name(), "i_1");
        auto& body_i_1 = loop_i_1->root();

        auto loop_j_1 = dynamic_cast<structured_control_flow::For*>(&body_i_1.at(0).first);
        EXPECT_TRUE(loop_j_1 != nullptr);
        EXPECT_EQ(loop_j_1->indvar()->get_name(), "j_1");

        auto loop_j_2 = dynamic_cast<structured_control_flow::For*>(&root.at(1).first);
        EXPECT_TRUE(loop_j_2 != nullptr);
        EXPECT_EQ(loop_j_2->indvar()->get_name(), "j_2");
        auto& body_j_2 = loop_j_2->root();

        auto loop_i_2 = dynamic_cast<structured_control_flow::For*>(&body_j_2.at(0).first);
        EXPECT_TRUE(loop_i_2 != nullptr);
        EXPECT_EQ(loop_i_2->indvar()->get_name(), "i_2");

        auto loop_i_3 = dynamic_cast<structured_control_flow::For*>(&root.at(2).first);
        EXPECT_TRUE(loop_i_3 != nullptr);
        EXPECT_EQ(loop_i_3->indvar()->get_name(), "i_3");

        auto loop_i_4 = dynamic_cast<structured_control_flow::For*>(&root.at(3).first);
        EXPECT_TRUE(loop_i_4 != nullptr);
        EXPECT_EQ(loop_i_4->indvar()->get_name(), "i_4");
        auto& body_i_4 = loop_i_4->root();

        auto loop_j_4 = dynamic_cast<structured_control_flow::For*>(&body_i_4.at(0).first);
        EXPECT_TRUE(loop_j_4 != nullptr);
        EXPECT_EQ(loop_j_4->indvar()->get_name(), "j_4");
    }
}

TEST(StrideMinimizationTest, Polybench_gesummv) {
    // Build SDFG
    auto init_sdfg = gesummv();
    auto schedule = std::make_unique<Schedule>(init_sdfg);

    // Pass
    passes::PerfectLoopDistribution pass;
    bool applies;
    do {
        applies = false;
        applies = pass.run(*schedule);
    } while (applies);

    passes::StrideMinimization stride_minimization_pass;
    do {
        applies = false;
        applies = stride_minimization_pass.run(*schedule);
        EXPECT_FALSE(applies);
    } while (applies);

    auto& sdfg = schedule->builder().subject();

    // Check
    /***
    for (i = 0; i < _PB_N; i++) {
        tmp[i] = SCALAR_VAL(0.0);
    }
    for (i = 0; i < _PB_N; i++) {
        y[i] = SCALAR_VAL(0.0);
    }
    for (i = 0; i < _PB_N; i++) {
        for (j = 0; j < _PB_N; j++) {
            tmp[i] = A[i][j] * x[j] + tmp[i];
            y[i] = B[i][j] * x[j] + y[i];
            }
    }
    for (i = 0; i < _PB_N; i++) {
        y[i] = tmp[i] + y[i];
    }
    */
}

TEST(StrideMinimizationTest, Polybench_syr2k) {
    // Build SDFG
    auto init_sdfg = syr2k();
    auto schedule = std::make_unique<Schedule>(init_sdfg);

    // Pass
    passes::PerfectLoopDistribution pass;
    bool applies;
    do {
        applies = false;
        applies = pass.run(*schedule);
    } while (applies);

    passes::StrideMinimization stride_minimization_pass;
    do {
        applies = false;
        applies = stride_minimization_pass.run(*schedule);
    } while (applies);

    auto& sdfg = schedule->builder().subject();

    // Check
    /***
    for (i = 0; i < _PB_N; i++) {
        for (j = 0; j <= i; j++) {
            C[i][j] *= beta;
        }
    }

    for (i = 0; i < _PB_N; i++) {
        for (j = 0; j <= i; j++) {
            for (k = 0; k < _PB_M; k++) {
                C[i][j] += A[j][k]*B[i][k] + B[j][k]*A[i][k];
            }
        }
    }
    ***/

    {
        auto& root = sdfg.root();
        EXPECT_EQ(root.size(), 2);

        auto loop_i_1 = dynamic_cast<structured_control_flow::For*>(&root.at(0).first);
        EXPECT_TRUE(loop_i_1 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_i_1->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_i_1->condition(),
                                  *symbolic::Lt(loop_i_1->indvar(), symbolic::symbol("N"))));
        EXPECT_TRUE(SymEngine::eq(*loop_i_1->update(),
                                  *symbolic::add(loop_i_1->indvar(), symbolic::integer(1))));
        EXPECT_EQ(loop_i_1->root().size(), 1);
        {
            auto loop_j_1 =
                dynamic_cast<structured_control_flow::For*>(&loop_i_1->root().at(0).first);
            EXPECT_TRUE(loop_j_1 != nullptr);
            EXPECT_TRUE(SymEngine::eq(*loop_j_1->init(), *symbolic::integer(0)));
            EXPECT_TRUE(SymEngine::eq(*loop_j_1->condition(),
                                      *symbolic::Le(loop_j_1->indvar(), loop_i_1->indvar())));
            EXPECT_TRUE(SymEngine::eq(*loop_j_1->update(),
                                      *symbolic::add(loop_j_1->indvar(), symbolic::integer(1))));
        }

        auto loop_i_2 = dynamic_cast<structured_control_flow::For*>(&root.at(1).first);
        EXPECT_TRUE(loop_i_2 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_i_2->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_i_2->condition(),
                                  *symbolic::Lt(loop_i_2->indvar(), symbolic::symbol("N"))));
        EXPECT_TRUE(SymEngine::eq(*loop_i_2->update(),
                                  *symbolic::add(loop_i_2->indvar(), symbolic::integer(1))));
        EXPECT_EQ(loop_i_2->root().size(), 1);
        {
            auto loop_j_2 =
                dynamic_cast<structured_control_flow::For*>(&loop_i_2->root().at(0).first);
            EXPECT_TRUE(loop_j_2 != nullptr);
            EXPECT_TRUE(SymEngine::eq(*loop_j_2->init(), *symbolic::integer(0)));
            EXPECT_TRUE(SymEngine::eq(*loop_j_2->condition(),
                                      *symbolic::Le(loop_j_2->indvar(), loop_i_2->indvar())));
            EXPECT_TRUE(SymEngine::eq(*loop_j_2->update(),
                                      *symbolic::add(loop_j_2->indvar(), symbolic::integer(1))));
            {
                auto loop_k =
                    dynamic_cast<structured_control_flow::For*>(&loop_j_2->root().at(0).first);
                EXPECT_TRUE(loop_k != nullptr);
                EXPECT_TRUE(SymEngine::eq(*loop_k->init(), *symbolic::integer(0)));
                EXPECT_TRUE(SymEngine::eq(*loop_k->condition(),
                                          *symbolic::Lt(loop_k->indvar(), symbolic::symbol("M"))));
                EXPECT_TRUE(SymEngine::eq(*loop_k->update(),
                                          *symbolic::add(loop_k->indvar(), symbolic::integer(1))));
            }
        }
    }
}

TEST(StrideMinimizationTest, Polybench_syrk) {
    auto init_sdfg = syrk();
    auto schedule = std::make_unique<Schedule>(init_sdfg);

    // Pass
    passes::PerfectLoopDistribution pass;
    bool applies;
    do {
        applies = false;
        applies = pass.run(*schedule);
    } while (applies);

    passes::StrideMinimization stride_minimization_pass;
    do {
        applies = false;
        applies = stride_minimization_pass.run(*schedule);
    } while (applies);

    auto& sdfg = schedule->builder().subject();

    // Check
    /***
     for (i = 0; i < _PB_N; i++) {
        for (j = 0; j <= i; j++)
            C[i][j] *= beta;
    }

    for (i = 0; i < _PB_N; i++) {
        for (j = 0; j <= i; j++)
            for (k = 0; k < _PB_M; k++) {
                C[i][j] += A[i][k] * A[j][k];
        }
    }
    ***/

    {
        auto& root = sdfg.root();
        EXPECT_EQ(root.size(), 2);

        auto loop_i = dynamic_cast<structured_control_flow::For*>(&root.at(0).first);
        EXPECT_TRUE(loop_i != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_i->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_i->condition(),
                                  *symbolic::Lt(loop_i->indvar(), symbolic::symbol("N"))));
        EXPECT_TRUE(SymEngine::eq(*loop_i->update(),
                                  *symbolic::add(loop_i->indvar(), symbolic::integer(1))));
        EXPECT_EQ(loop_i->root().size(), 1);
        {
            auto loop_j_1 =
                dynamic_cast<structured_control_flow::For*>(&loop_i->root().at(0).first);
            EXPECT_TRUE(loop_j_1 != nullptr);
            EXPECT_TRUE(SymEngine::eq(*loop_j_1->init(), *symbolic::integer(0)));
            EXPECT_TRUE(SymEngine::eq(*loop_j_1->condition(),
                                      *symbolic::Le(loop_j_1->indvar(), loop_i->indvar())));
            EXPECT_TRUE(SymEngine::eq(*loop_j_1->update(),
                                      *symbolic::add(loop_j_1->indvar(), symbolic::integer(1))));
        }

        auto loop_i_2 = dynamic_cast<structured_control_flow::For*>(&root.at(1).first);
        EXPECT_TRUE(loop_i_2 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_i_2->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_i_2->condition(),
                                  *symbolic::Lt(loop_i_2->indvar(), symbolic::symbol("N"))));
        EXPECT_TRUE(SymEngine::eq(*loop_i_2->update(),
                                  *symbolic::add(loop_i_2->indvar(), symbolic::integer(1))));
        EXPECT_EQ(loop_i_2->root().size(), 1);
        {
            auto loop_j_2 =
                dynamic_cast<structured_control_flow::For*>(&loop_i_2->root().at(0).first);
            EXPECT_TRUE(loop_j_2 != nullptr);
            EXPECT_TRUE(SymEngine::eq(*loop_j_2->init(), *symbolic::integer(0)));
            EXPECT_TRUE(SymEngine::eq(*loop_j_2->condition(),
                                      *symbolic::Le(loop_j_2->indvar(), loop_i_2->indvar())));
            EXPECT_TRUE(SymEngine::eq(*loop_j_2->update(),
                                      *symbolic::add(loop_j_2->indvar(), symbolic::integer(1))));
            {
                auto loop_k =
                    dynamic_cast<structured_control_flow::For*>(&loop_j_2->root().at(0).first);
                EXPECT_TRUE(loop_k != nullptr);
                EXPECT_TRUE(SymEngine::eq(*loop_k->init(), *symbolic::integer(0)));
                EXPECT_TRUE(SymEngine::eq(*loop_k->condition(),
                                          *symbolic::Lt(loop_k->indvar(), symbolic::symbol("M"))));
                EXPECT_TRUE(SymEngine::eq(*loop_k->update(),
                                          *symbolic::add(loop_k->indvar(), symbolic::integer(1))));
            }
        }
    }
}

TEST(StrideMinimizationTest, Polybench_trmm) {
    // Build SDFG
    auto init_sdfg = trmm();
    auto schedule = std::make_unique<Schedule>(init_sdfg);

    // Pass
    passes::PerfectLoopDistribution pass;
    bool applies;
    do {
        applies = false;
        applies = pass.run(*schedule);
    } while (applies);

    passes::StrideMinimization stride_minimization_pass;
    do {
        applies = false;
        applies = stride_minimization_pass.run(*schedule);
    } while (applies);

    auto& sdfg = schedule->builder().subject();

    // Check
    /***
     for (i = 0; i < _PB_M; i++) {
        for (k = i+1; k < _PB_M; k++) {
            for (j = 0; j < _PB_N; j++) {
                B[i][j] += A[k][i] * B[k][j];
            }
        }
        for (j = 0; j < _PB_N; j++) {
            B[i][j] = alpha * B[i][j];
        }
    }
    */

    {
        auto& root = sdfg.root();
        EXPECT_EQ(root.size(), 1);

        auto loop_i_1 = dynamic_cast<structured_control_flow::For*>(&root.at(0).first);
        EXPECT_TRUE(loop_i_1 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_i_1->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_i_1->condition(),
                                  *symbolic::Lt(loop_i_1->indvar(), symbolic::symbol("M"))));
        EXPECT_TRUE(SymEngine::eq(*loop_i_1->update(),
                                  *symbolic::add(loop_i_1->indvar(), symbolic::integer(1))));
        EXPECT_EQ(loop_i_1->root().size(), 2);
        {
            auto loop_k_1 =
                dynamic_cast<structured_control_flow::For*>(&loop_i_1->root().at(0).first);
            EXPECT_TRUE(loop_k_1 != nullptr);
            EXPECT_TRUE(SymEngine::eq(*loop_k_1->init(),
                                      *symbolic::add(loop_i_1->indvar(), symbolic::integer(1))));
            EXPECT_TRUE(SymEngine::eq(*loop_k_1->condition(),
                                      *symbolic::Lt(loop_k_1->indvar(), symbolic::symbol("M"))));
            EXPECT_TRUE(SymEngine::eq(*loop_k_1->update(),
                                      *symbolic::add(loop_k_1->indvar(), symbolic::integer(1))));
            {
                auto loop_j_1 =
                    dynamic_cast<structured_control_flow::For*>(&loop_k_1->root().at(0).first);
                EXPECT_TRUE(loop_j_1 != nullptr);
                EXPECT_TRUE(SymEngine::eq(*loop_j_1->init(), *symbolic::integer(0)));
                EXPECT_TRUE(
                    SymEngine::eq(*loop_j_1->condition(),
                                  *symbolic::Lt(loop_j_1->indvar(), symbolic::symbol("N"))));
                EXPECT_TRUE(SymEngine::eq(
                    *loop_j_1->update(), *symbolic::add(loop_j_1->indvar(), symbolic::integer(1))));
            }

            auto loop_j_2 =
                dynamic_cast<structured_control_flow::For*>(&loop_i_1->root().at(1).first);
            EXPECT_TRUE(loop_j_2 != nullptr);
            EXPECT_TRUE(SymEngine::eq(*loop_j_2->init(), *symbolic::integer(0)));
            EXPECT_TRUE(SymEngine::eq(*loop_j_2->condition(),
                                      *symbolic::Lt(loop_j_2->indvar(), symbolic::symbol("N"))));
            EXPECT_TRUE(SymEngine::eq(*loop_j_2->update(),
                                      *symbolic::add(loop_j_2->indvar(), symbolic::integer(1))));
        }
    }
}

TEST(StrideMinimizationTest, Polybench_atax) {
    // Build SDFG
    auto init_sdfg = atax();
    auto schedule = std::make_unique<Schedule>(init_sdfg);

    // Pass
    passes::PerfectLoopDistribution pass;
    bool applies;
    do {
        applies = false;
        applies = pass.run(*schedule);
    } while (applies);

    passes::StrideMinimization stride_minimization_pass;
    do {
        applies = false;
        applies = stride_minimization_pass.run(*schedule);
        EXPECT_FALSE(applies);
    } while (applies);

    auto& sdfg = schedule->builder().subject();

    // Check
    /***
    for (i = 0; i < _PB_N; i++)
        y[i] = 0;

    for (i = 0; i < _PB_M; i++) {
        tmp[i] = SCALAR_VAL(0.0);
    }

    for (i = 0; i < _PB_M; i++) {
        for (j = 0; j < _PB_N; j++)
            tmp[i] = tmp[i] + A[i][j] * x[j];
    }

    for (i = 0; i < _PB_M; i++) {
        for (j = 0; j < _PB_N; j++)
            y[j] = y[j] + A[i][j] * tmp[i];
    }
    */
}

TEST(StrideMinimizationTest, Polybench_bicg) {
    // Build SDFG
    auto init_sdfg = bicg();
    auto schedule = std::make_unique<Schedule>(init_sdfg);

    // Pass
    passes::PerfectLoopDistribution pass;
    bool applies;
    do {
        applies = false;
        applies = pass.run(*schedule);
    } while (applies);

    passes::StrideMinimization stride_minimization_pass;
    do {
        applies = false;
        applies = stride_minimization_pass.run(*schedule);
        EXPECT_FALSE(applies);
    } while (applies);

    auto& sdfg = schedule->builder().subject();

    // Check
    /***
        for (i = 0; i < _PB_M; i++)
            s[i] = 0;
        for (i = 0; i < _PB_N; i++)
            q[i] = SCALAR_VAL(0.0);

        for (i = 0; i < _PB_N; i++) {
            for (j = 0; j < _PB_M; j++) {
                s[j] = s[j] + r[i] * A[i][j];
                q[i] = q[i] + A[i][j] * p[j];
            }
        }
    */
}

TEST(StrideMinimizationTest, Polybench_doitgen) {
    // Build SDFG
    auto init_sdfg = doitgen();
    auto schedule = std::make_unique<Schedule>(init_sdfg);

    // Pass
    passes::PerfectLoopDistribution pass;
    bool applies;
    do {
        applies = false;
        applies = pass.run(*schedule);
    } while (applies);

    passes::StrideMinimization stride_minimization_pass;
    do {
        applies = false;
        applies = stride_minimization_pass.run(*schedule);
    } while (applies);

    auto& sdfg = schedule->builder().subject();

    // Check
    /***
    for (r = 0; r < _PB_NR; r++) {}
        for (q = 0; q < _PB_NQ; q++)  {
            for (p = 0; p < _PB_NP; p++)  {
                    sum[p] = SCALAR_VAL(0.0);
            }
                for (s = 0; s < _PB_NP; s++) {
                for (p = 0; p < _PB_NP; p++)
                        sum[p] += A[r][q][s] * C4[s][p];
            }
            for (p = 0; p < _PB_NP; p++)
                    A[r][q][p] = sum[p];
        }
    }
    */

    {
        auto& root = sdfg.root();
        EXPECT_EQ(root.size(), 1);

        auto loop_r = dynamic_cast<structured_control_flow::For*>(&root.at(0).first);
        EXPECT_TRUE(loop_r != nullptr);
        EXPECT_EQ(loop_r->indvar()->get_name(), "r");
        auto& body_r = loop_r->root();

        auto loop_q = dynamic_cast<structured_control_flow::For*>(&body_r.at(0).first);
        EXPECT_TRUE(loop_q != nullptr);
        EXPECT_EQ(loop_q->indvar()->get_name(), "q");
        auto& body_q = loop_q->root();

        auto loop_p_1 = dynamic_cast<structured_control_flow::For*>(&body_q.at(0).first);
        EXPECT_TRUE(loop_p_1 != nullptr);
        EXPECT_EQ(loop_p_1->indvar()->get_name(), "p_10");
        auto& body_p_1 = loop_p_1->root();

        auto loop_s = dynamic_cast<structured_control_flow::For*>(&body_q.at(1).first);
        EXPECT_TRUE(loop_s != nullptr);
        EXPECT_EQ(loop_s->indvar()->get_name(), "s");
        auto& body_s = loop_s->root();

        auto loop_p_2 = dynamic_cast<structured_control_flow::For*>(&body_s.at(0).first);
        EXPECT_TRUE(loop_p_2 != nullptr);
        EXPECT_EQ(loop_p_2->indvar()->get_name(), "p_1");

        auto loop_p_3 = dynamic_cast<structured_control_flow::For*>(&body_q.at(2).first);
        EXPECT_TRUE(loop_p_3 != nullptr);
        EXPECT_EQ(loop_p_3->indvar()->get_name(), "p_2");
    }
}

TEST(StrideMinimizationTest, Polybench_mvt) {
    // Build SDFG
    auto init_sdfg = mvt();
    auto schedule = std::make_unique<Schedule>(init_sdfg);

    // Pass
    passes::PerfectLoopDistribution pass;
    bool applies;
    do {
        applies = false;
        applies = pass.run(*schedule);
    } while (applies);

    passes::StrideMinimization stride_minimization_pass;
    do {
        applies = false;
        applies = stride_minimization_pass.run(*schedule);
    } while (applies);

    auto& sdfg = schedule->builder().subject();

    // Check
    /***
    for (i = 0; i < _PB_N; i++)
        for (j = 0; j < _PB_N; j++)
            x1[i] = x1[i] + A[i][j] * y_1[j];
    for (j = 0; j < _PB_N; j++)
        for (i = 0; i < _PB_N; i++)
            x2[i] = x2[i] + A[j][i] * y_2[j];
    */

    {
        auto& root = sdfg.root();
        EXPECT_EQ(root.size(), 2);

        auto loop_i_1 = dynamic_cast<structured_control_flow::For*>(&root.at(0).first);
        EXPECT_TRUE(loop_i_1 != nullptr);
        EXPECT_TRUE(loop_i_1->indvar()->get_name() == "i_1");
        auto& body_loop_i_1 = loop_i_1->root();

        auto loop_j_1 = dynamic_cast<structured_control_flow::For*>(&body_loop_i_1.at(0).first);
        EXPECT_TRUE(loop_j_1 != nullptr);
        EXPECT_TRUE(loop_j_1->indvar()->get_name() == "j_1");

        auto loop_i_2 = dynamic_cast<structured_control_flow::For*>(&root.at(1).first);
        EXPECT_TRUE(loop_i_2 != nullptr);
        EXPECT_TRUE(loop_i_2->indvar()->get_name() == "j_2");
        auto& body_loop_i_2 = loop_i_2->root();

        auto loop_j_2 = dynamic_cast<structured_control_flow::For*>(&body_loop_i_2.at(0).first);
        EXPECT_TRUE(loop_j_2 != nullptr);
        EXPECT_TRUE(loop_j_2->indvar()->get_name() == "i_2");
    }
}

TEST(StrideMinimizationTest, Polybench_cholesky) {
    // Build SDFG
    auto init_sdfg = cholesky();
    auto schedule = std::make_unique<Schedule>(init_sdfg);
    ;

    // Pass
    passes::PerfectLoopDistribution pass;
    bool applies;
    do {
        applies = false;
        applies = pass.run(*schedule);
    } while (applies);

    passes::StrideMinimization stride_minimization_pass;
    do {
        applies = false;
        applies = stride_minimization_pass.run(*schedule);
    } while (applies);

    auto& sdfg = schedule->builder().subject();

    // Check
    /***
     for (i = 0; i < _PB_N; i++) {
        //j<i
        for (j = 0; j < i; j++) {
            for (k = 0; k < j; k++) {
                A[i][j] -= A[i][k] * A[j][k];
            }

            A[i][j] /= A[j][j];
        }
        // i==j case
        for (k = 0; k < i; k++) {
            A[i][i] -= A[i][k] * A[i][k];
        }
        A[i][i] = sqrt(A[i][i]);
    }
    */

    {
        auto& root = sdfg.root();
        EXPECT_EQ(root.size(), 1);

        auto loop_i = dynamic_cast<structured_control_flow::For*>(&root.at(0).first);
        EXPECT_TRUE(loop_i != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_i->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_i->condition(),
                                  *symbolic::Lt(loop_i->indvar(), symbolic::symbol("N"))));
        EXPECT_TRUE(SymEngine::eq(*loop_i->update(),
                                  *symbolic::add(loop_i->indvar(), symbolic::integer(1))));
        auto& body_i = loop_i->root();
        EXPECT_EQ(body_i.size(), 3);

        auto loop_j_1 = dynamic_cast<structured_control_flow::For*>(&body_i.at(0).first);
        EXPECT_TRUE(loop_j_1 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_j_1->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_j_1->condition(),
                                  *symbolic::Lt(loop_j_1->indvar(), loop_i->indvar())));
        EXPECT_TRUE(SymEngine::eq(*loop_j_1->update(),
                                  *symbolic::add(loop_j_1->indvar(), symbolic::integer(1))));
        auto& body_j_1 = loop_j_1->root();
        EXPECT_EQ(body_j_1.size(), 2);

        auto loop_k_1 = dynamic_cast<structured_control_flow::For*>(&body_j_1.at(0).first);
        EXPECT_TRUE(loop_k_1 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_k_1->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_k_1->condition(),
                                  *symbolic::Lt(loop_k_1->indvar(), loop_j_1->indvar())));
        EXPECT_TRUE(SymEngine::eq(*loop_k_1->update(),
                                  *symbolic::add(loop_k_1->indvar(), symbolic::integer(1))));
        auto& body_k_1 = loop_k_1->root();

        auto loop_k_2 = dynamic_cast<structured_control_flow::For*>(&body_i.at(1).first);
        EXPECT_TRUE(loop_k_2 != nullptr);
        EXPECT_TRUE(SymEngine::eq(*loop_k_2->init(), *symbolic::integer(0)));
        EXPECT_TRUE(SymEngine::eq(*loop_k_2->condition(),
                                  *symbolic::Lt(loop_k_2->indvar(), loop_i->indvar())));
        EXPECT_TRUE(SymEngine::eq(*loop_k_2->update(),
                                  *symbolic::add(loop_k_2->indvar(), symbolic::integer(1))));
        auto& body_j_2 = loop_k_2->root();
    }
}

TEST(StrideMinimizationTest, Polybench_fdtd_2d) {
    // Build SDFG
    auto init_sdfg = fdtd_2d();
    auto schedule = std::make_unique<Schedule>(init_sdfg);

    // Pass
    passes::PerfectLoopDistribution pass;
    bool applies;
    do {
        applies = false;
        applies = pass.run(*schedule);
    } while (applies);

    passes::StrideMinimization stride_minimization_pass;
    do {
        applies = false;
        applies = stride_minimization_pass.run(*schedule);
        EXPECT_FALSE(applies);
    } while (applies);

    auto& sdfg = schedule->builder().subject();
}

TEST(StrideMinimizationTest, Rodinia_srad) {
    // Build SDFG
    auto init_sdfg = srad();
    auto schedule = std::make_unique<Schedule>(init_sdfg);

    // Pass
    passes::PerfectLoopDistribution pass;
    bool applies;
    do {
        applies = false;
        applies = pass.run(*schedule);
    } while (applies);

    passes::StrideMinimization stride_minimization_pass;
    do {
        applies = false;
        applies = stride_minimization_pass.run(*schedule);
        EXPECT_FALSE(applies);
    } while (applies);

    auto& sdfg = schedule->builder().subject();
}
