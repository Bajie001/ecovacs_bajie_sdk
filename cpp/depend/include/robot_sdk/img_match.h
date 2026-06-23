#ifndef ROBOT_SDK_IMG_MATCH_H_
#define ROBOT_SDK_IMG_MATCH_H_

#include "export.h"
#include "types.h"
#include <memory>
#include <string>
#include <vector>

namespace robot_sdk {

/**
 * 批量图像特征提取：POST /image_embedding_extern。
 * 默认基地址为 ``http://$ECO_ROBOT_HOST:6002``。
 */
ROBOT_SDK_PUBLIC std::unique_ptr<ImageEmbeddingResponse> eco_ImageEmbeddingExtract(
    const ImageEmbeddingRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC std::unique_ptr<ImageEmbeddingResponse> eco_ImageEmbeddingExtract(
    const ImageEmbeddingRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_ImageEmbeddingExtractSimple(
    const ImageEmbeddingRequest& req,
    std::vector<std::vector<float>>& embeddings,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_ImageEmbeddingExtractSimple(
    const ImageEmbeddingRequest& req,
    std::vector<std::vector<float>>& embeddings,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

/**
 * fused 余弦相似度：POST /fused_similarity_hybrid_extern。同侧须 image 与 embedding 二选一。
 * 默认基地址为 ``http://$ECO_ROBOT_HOST:6002``。
 */
ROBOT_SDK_PUBLIC std::unique_ptr<FusedSimilarityResponse> eco_FusedSimilarityHybrid(
    const HybridFusedSimRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC std::unique_ptr<FusedSimilarityResponse> eco_FusedSimilarityHybrid(
    const HybridFusedSimRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_FusedSimilarityHybridSimple(
    const HybridFusedSimRequest& req,
    double& similarity,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_FusedSimilarityHybridSimple(
    const HybridFusedSimRequest& req,
    double& similarity,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

/**
 * 标签优先 + 视觉成对匹配：POST /pairwise_v2
 * 默认基地址为 ``http://$ECO_ROBOT_HOST:6002``。
 */
ROBOT_SDK_PUBLIC std::unique_ptr<PairwiseResponse> eco_PairwiseMatch(
    const PairwiseRequest& req,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC std::unique_ptr<PairwiseResponse> eco_PairwiseMatch(
    const PairwiseRequest& req,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_PairwiseMatchSimple(
    const PairwiseRequest& req,
    std::vector<PairwiseMatchedItem>& items,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_PairwiseMatchSimple(
    const PairwiseRequest& req,
    std::vector<PairwiseMatchedItem>& items,
    std::string& error_msg,
    const std::string& base_url,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

/**
 * 跨视图匹配目标框：在目标检测列表中找到与参考框最相似的一项。
 * 调用了eco_ImageEmbeddingExtractSimple、eco_FusedSimilarityHybridSimple
 */
ROBOT_SDK_PUBLIC bool eco_MatchObjViews(
    const robot_sdk::ImageQueryNotifyData& head_image,
    const std::vector<int>& ref_bbox,
    const robot_sdk::ImageQueryNotifyData& arm_image,
    const std::vector<robot_sdk::ObjectDetection>& arm_items,
    robot_sdk::ObjectDetection& out,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

ROBOT_SDK_PUBLIC bool eco_MatchObjViews(
    const robot_sdk::ImageQueryNotifyData& head_image,
    const std::vector<int>& ref_bbox,
    const robot_sdk::ImageQueryNotifyData& arm_image,
    const std::vector<robot_sdk::ObjectDetection>& arm_items,
    robot_sdk::ObjectDetection& out,
    std::string& error_msg,
    int timeout_ms = kDefaultHttpRequestTimeoutMs);

}  // namespace robot_sdk

#endif
