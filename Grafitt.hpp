#pragma once

// grafitt.hpp
// Header-only graph library inspired by OCamlGraph, with:
// - immutable/persistent and mutable/imperative graphs
// - OCamlGraph-like traversal/fold/builder style APIs
// - Queryfitt query DSL scaffolding
// - graph rewriting
// - GBIN serialization hooks for SerdeTk
//
// Requires C++20.

#include <algorithm>
#include <cctype>
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <istream>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <queue>
#include <ranges>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#if __has_include("DSLUtils.hpp")
#  include "DSLUtils.hpp"
#  define GRAFITT_HAS_DSLUTILS 1
#else
#  define GRAFITT_HAS_DSLUTILS 0
#endif

#if __has_include("SerdeTk.hpp")
#  include "SerdeTk.hpp"
#  define GRAFITT_HAS_SERDETK 1
#else
#  define GRAFITT_HAS_SERDETK 0
#endif

#if __has_include("matcheroni/Matcheroni.hpp") && __has_include("matcheroni/Utilities.hpp")
#  include "matcheroni/Matcheroni.hpp"
#  include "matcheroni/Utilities.hpp"
#  define GRAFITT_HAS_MATCHERONI 1
#else
#  define GRAFITT_HAS_MATCHERONI 0
#endif

namespace grafitt {

// ============================================================
// Core utility concepts and traits
// ============================================================

template<class T>
concept Hashable = requires(T v) {
    { std::hash<T>{}(v) } -> std::convertible_to<std::size_t>;
};

template<class T>
concept EqualityComparable = requires(T a, T b) {
    { a == b } -> std::convertible_to<bool>;
};

template<class V>
concept VertexLike = Hashable<V> && EqualityComparable<V>;

template<class E>
concept EdgeLabelLike = std::default_initializable<E>;

struct unit final {
    friend constexpr bool operator==(unit, unit) = default;
};

template<class T>
struct identity_key {
    using type = T;
    constexpr const T& operator()(const T& v) const noexcept { return v; }
};

template<class T>
inline std::string to_string_fallback(const T& value) {
    if constexpr (requires(std::ostream& os) { os << value; }) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    } else {
        return "<unprintable>";
    }
}

// ============================================================
// Exceptions
// ============================================================

struct grafitt_error : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct parse_error : grafitt_error {
    using grafitt_error::grafitt_error;
};

struct serialization_error : grafitt_error {
    using grafitt_error::grafitt_error;
};

struct rewrite_error : grafitt_error {
    using grafitt_error::grafitt_error;
};

struct query_error : grafitt_error {
    using grafitt_error::grafitt_error;
};

// ============================================================
// Edge model
// ============================================================

template<class Vertex, class EdgeLabel = unit>
struct edge {
    Vertex src {};
    Vertex dst {};
    EdgeLabel label {};

    friend bool operator==(const edge&, const edge&) = default;
};

template<class Vertex, class EdgeLabel>
struct edge_hash {
    std::size_t operator()(const edge<Vertex, EdgeLabel>& e) const noexcept {
        std::size_t h1 = std::hash<Vertex>{}(e.src);
        std::size_t h2 = std::hash<Vertex>{}(e.dst);
        std::size_t h3 = 0;
        if constexpr (Hashable<EdgeLabel>) {
            h3 = std::hash<EdgeLabel>{}(e.label);
        }
        return h1 ^ (h2 << 1) ^ (h3 << 7);
    }
};

// ============================================================
// Graph traits and direction
// ============================================================

enum class direction {
    directed,
    undirected
};

template<class Vertex, class EdgeLabel = unit>
using edge_set = std::unordered_set<edge<Vertex, EdgeLabel>, edge_hash<Vertex, EdgeLabel>>;

// ============================================================
// Internal graph storage
// ============================================================

template<VertexLike Vertex, class EdgeLabel = unit>
class adjacency_storage {
public:
    using vertex_type = Vertex;
    using edge_label_type = EdgeLabel;
    using edge_type = edge<Vertex, EdgeLabel>;

private:
    direction dir_ { direction::directed };
    std::unordered_set<Vertex> vertices_;
    edge_set<Vertex, EdgeLabel> edges_;
    std::unordered_map<Vertex, std::vector<edge_type>> out_;
    std::unordered_map<Vertex, std::vector<edge_type>> in_;

public:
    adjacency_storage() = default;
    explicit adjacency_storage(direction dir) : dir_(dir) {}

    [[nodiscard]] direction dir() const noexcept { return dir_; }

    [[nodiscard]] bool mem_vertex(const Vertex& v) const {
        return vertices_.contains(v);
    }

    [[nodiscard]] bool mem_edge(const Vertex& s, const Vertex& d) const {
        if (auto it = out_.find(s); it != out_.end()) {
            for (const auto& e : it->second) {
                if (e.dst == d) return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool mem_edge_e(const edge_type& e) const {
        return edges_.contains(e);
    }

    void add_vertex(const Vertex& v) {
        vertices_.insert(v);
        out_.try_emplace(v);
        in_.try_emplace(v);
    }

    void add_edge(const edge_type& e) {
        add_vertex(e.src);
        add_vertex(e.dst);

        if (edges_.contains(e)) return;

        edges_.insert(e);
        out_[e.src].push_back(e);
        in_[e.dst].push_back(e);

        if (dir_ == direction::undirected && !(e.src == e.dst)) {
            edge_type rev { e.dst, e.src, e.label };
            if (!edges_.contains(rev)) {
                edges_.insert(rev);
                out_[e.dst].push_back(rev);
                in_[e.src].push_back(rev);
            }
        }
    }

    void remove_edge(const edge_type& e) {
        auto erase_from = [](auto& vec, const edge_type& x) {
            vec.erase(std::remove(vec.begin(), vec.end(), x), vec.end());
        };

        if (edges_.erase(e)) {
            erase_from(out_[e.src], e);
            erase_from(in_[e.dst], e);
        }

        if (dir_ == direction::undirected && !(e.src == e.dst)) {
            edge_type rev { e.dst, e.src, e.label };
            if (edges_.erase(rev)) {
                erase_from(out_[rev.src], rev);
                erase_from(in_[rev.dst], rev);
            }
        }
    }

    void remove_vertex(const Vertex& v) {
        if (!vertices_.contains(v)) return;

        auto outgoing = out_[v];
        auto incoming = in_[v];

        for (const auto& e : outgoing) remove_edge(e);
        for (const auto& e : incoming) remove_edge(e);

        out_.erase(v);
        in_.erase(v);
        vertices_.erase(v);
    }

    [[nodiscard]] const std::unordered_set<Vertex>& vertices() const noexcept { return vertices_; }
    [[nodiscard]] const edge_set<Vertex, EdgeLabel>& edges() const noexcept { return edges_; }

    [[nodiscard]] std::vector<edge_type> out_edges(const Vertex& v) const {
        if (auto it = out_.find(v); it != out_.end()) return it->second;
        return {};
    }

    [[nodiscard]] std::vector<edge_type> in_edges(const Vertex& v) const {
        if (auto it = in_.find(v); it != in_.end()) return it->second;
        return {};
    }

    [[nodiscard]] std::vector<edge_type> all_edges(const Vertex& s, const Vertex& d) const {
        std::vector<edge_type> r;
        if (auto it = out_.find(s); it != out_.end()) {
            for (const auto& e : it->second) {
                if (e.dst == d) r.push_back(e);
            }
        }
        return r;
    }

    [[nodiscard]] std::size_t nb_vertex() const noexcept { return vertices_.size(); }
    [[nodiscard]] std::size_t nb_edges() const noexcept { return edges_.size(); }
};

// ============================================================
// Common graph API base
// ============================================================

template<VertexLike Vertex, class EdgeLabel = unit>
class graph_view_base {
public:
    using vertex_type = Vertex;
    using edge_label_type = EdgeLabel;
    using edge_type = edge<Vertex, EdgeLabel>;
    using storage_type = adjacency_storage<Vertex, EdgeLabel>;

protected:
    std::shared_ptr<const storage_type> storage_;

    explicit graph_view_base(std::shared_ptr<const storage_type> s)
        : storage_(std::move(s)) {}

public:
    graph_view_base() : storage_(std::make_shared<storage_type>()) {}

    [[nodiscard]] direction dir() const noexcept { return storage_->dir(); }
    [[nodiscard]] bool is_directed() const noexcept { return dir() == direction::directed; }

    [[nodiscard]] bool mem_vertex(const Vertex& v) const { return storage_->mem_vertex(v); }
    [[nodiscard]] bool mem_edge(const Vertex& s, const Vertex& d) const { return storage_->mem_edge(s, d); }
    [[nodiscard]] bool mem_edge_e(const edge_type& e) const { return storage_->mem_edge_e(e); }

    [[nodiscard]] std::size_t nb_vertex() const noexcept { return storage_->nb_vertex(); }
    [[nodiscard]] std::size_t nb_edges() const noexcept { return storage_->nb_edges(); }

    template<class F>
    void iter_vertex(F&& f) const {
        for (const auto& v : storage_->vertices()) std::invoke(f, v);
    }

    template<class F, class Acc>
    [[nodiscard]] Acc fold_vertex(F&& f, Acc init) const {
        for (const auto& v : storage_->vertices()) {
            init = std::invoke(f, v, std::move(init));
        }
        return init;
    }

    template<class F>
    void iter_edges(F&& f) const {
        for (const auto& e : storage_->edges()) std::invoke(f, e.src, e.dst);
    }

    template<class F>
    void iter_edges_e(F&& f) const {
        for (const auto& e : storage_->edges()) std::invoke(f, e);
    }

    template<class F, class Acc>
    [[nodiscard]] Acc fold_edges(F&& f, Acc init) const {
        for (const auto& e : storage_->edges()) {
            init = std::invoke(f, e.src, e.dst, std::move(init));
        }
        return init;
    }

    template<class F, class Acc>
    [[nodiscard]] Acc fold_edges_e(F&& f, Acc init) const {
        for (const auto& e : storage_->edges()) {
            init = std::invoke(f, e, std::move(init));
        }
        return init;
    }

    template<class F>
    void iter_succ(const Vertex& v, F&& f) const {
        for (const auto& e : storage_->out_edges(v)) std::invoke(f, e.dst);
    }

    template<class F>
    void iter_pred(const Vertex& v, F&& f) const {
        for (const auto& e : storage_->in_edges(v)) std::invoke(f, e.src);
    }

    template<class F>
    void iter_succ_e(const Vertex& v, F&& f) const {
        for (const auto& e : storage_->out_edges(v)) std::invoke(f, e);
    }

    template<class F>
    void iter_pred_e(const Vertex& v, F&& f) const {
        for (const auto& e : storage_->in_edges(v)) std::invoke(f, e);
    }

    [[nodiscard]] std::vector<edge_type> succ_e(const Vertex& v) const {
        return storage_->out_edges(v);
    }

    [[nodiscard]] std::vector<edge_type> pred_e(const Vertex& v) const {
        return storage_->in_edges(v);
    }

    [[nodiscard]] std::vector<Vertex> succ(const Vertex& v) const {
        std::vector<Vertex> r;
        for (const auto& e : storage_->out_edges(v)) r.push_back(e.dst);
        return r;
    }

    [[nodiscard]] std::vector<Vertex> pred(const Vertex& v) const {
        std::vector<Vertex> r;
        for (const auto& e : storage_->in_edges(v)) r.push_back(e.src);
        return r;
    }

    [[nodiscard]] std::vector<edge_type> find_all_edges(const Vertex& s, const Vertex& d) const {
        return storage_->all_edges(s, d);
    }

    [[nodiscard]] const storage_type& storage() const noexcept { return *storage_; }
};

// ============================================================
// Persistent / immutable graph
// Similar to OCamlGraph Sig.P spirit
// ============================================================

template<VertexLike Vertex, class EdgeLabel = unit>
class persistent_graph final : public graph_view_base<Vertex, EdgeLabel> {
public:
    using base = graph_view_base<Vertex, EdgeLabel>;
    using typename base::edge_type;
    using typename base::storage_type;

    persistent_graph()
        : base(std::make_shared<storage_type>()) {}

    explicit persistent_graph(direction dir)
        : base(std::make_shared<storage_type>(dir)) {}

private:
    explicit persistent_graph(std::shared_ptr<const storage_type> s)
        : base(std::move(s)) {}

public:
    [[nodiscard]] persistent_graph add_vertex(const Vertex& v) const {
        auto copy = std::make_shared<storage_type>(this->storage());
        copy->add_vertex(v);
        return persistent_graph(copy);
    }

    [[nodiscard]] persistent_graph add_edge(const Vertex& src, const Vertex& dst, const EdgeLabel& label = {}) const {
        auto copy = std::make_shared<storage_type>(this->storage());
        copy->add_edge(edge_type{src, dst, label});
        return persistent_graph(copy);
    }

    [[nodiscard]] persistent_graph add_edge_e(const edge_type& e) const {
        auto copy = std::make_shared<storage_type>(this->storage());
        copy->add_edge(e);
        return persistent_graph(copy);
    }

    [[nodiscard]] persistent_graph remove_vertex(const Vertex& v) const {
        auto copy = std::make_shared<storage_type>(this->storage());
        copy->remove_vertex(v);
        return persistent_graph(copy);
    }

    [[nodiscard]] persistent_graph remove_edge(const Vertex& src, const Vertex& dst, const EdgeLabel& label = {}) const {
        auto copy = std::make_shared<storage_type>(this->storage());
        copy->remove_edge(edge_type{src, dst, label});
        return persistent_graph(copy);
    }

    [[nodiscard]] persistent_graph remove_edge_e(const edge_type& e) const {
        auto copy = std::make_shared<storage_type>(this->storage());
        copy->remove_edge(e);
        return persistent_graph(copy);
    }
};

// ============================================================
// Imperative / mutable graph
// Similar to OCamlGraph Sig.I spirit
// ============================================================

template<VertexLike Vertex, class EdgeLabel = unit>
class imperative_graph final {
public:
    using vertex_type = Vertex;
    using edge_label_type = EdgeLabel;
    using edge_type = edge<Vertex, EdgeLabel>;
    using storage_type = adjacency_storage<Vertex, EdgeLabel>;

private:
    storage_type storage_;

public:
    imperative_graph() = default;
    explicit imperative_graph(direction dir) : storage_(dir) {}

    [[nodiscard]] direction dir() const noexcept { return storage_.dir(); }
    [[nodiscard]] bool is_directed() const noexcept { return dir() == direction::directed; }

    void add_vertex(const Vertex& v) { storage_.add_vertex(v); }
    void add_edge(const Vertex& src, const Vertex& dst, const EdgeLabel& label = {}) {
        storage_.add_edge(edge_type{src, dst, label});
    }
    void add_edge_e(const edge_type& e) { storage_.add_edge(e); }

    void remove_vertex(const Vertex& v) { storage_.remove_vertex(v); }
    void remove_edge(const Vertex& src, const Vertex& dst, const EdgeLabel& label = {}) {
        storage_.remove_edge(edge_type{src, dst, label});
    }
    void remove_edge_e(const edge_type& e) { storage_.remove_edge(e); }

    [[nodiscard]] bool mem_vertex(const Vertex& v) const { return storage_.mem_vertex(v); }
    [[nodiscard]] bool mem_edge(const Vertex& s, const Vertex& d) const { return storage_.mem_edge(s, d); }
    [[nodiscard]] bool mem_edge_e(const edge_type& e) const { return storage_.mem_edge_e(e); }

    [[nodiscard]] std::size_t nb_vertex() const noexcept { return storage_.nb_vertex(); }
    [[nodiscard]] std::size_t nb_edges() const noexcept { return storage_.nb_edges(); }

    template<class F>
    void iter_vertex(F&& f) const {
        for (const auto& v : storage_.vertices()) std::invoke(f, v);
    }

    template<class F, class Acc>
    [[nodiscard]] Acc fold_vertex(F&& f, Acc init) const {
        for (const auto& v : storage_.vertices()) {
            init = std::invoke(f, v, std::move(init));
        }
        return init;
    }

    template<class F>
    void iter_edges(F&& f) const {
        for (const auto& e : storage_.edges()) std::invoke(f, e.src, e.dst);
    }

    template<class F>
    void iter_edges_e(F&& f) const {
        for (const auto& e : storage_.edges()) std::invoke(f, e);
    }

    template<class F, class Acc>
    [[nodiscard]] Acc fold_edges(F&& f, Acc init) const {
        for (const auto& e : storage_.edges()) {
            init = std::invoke(f, e.src, e.dst, std::move(init));
        }
        return init;
    }

    template<class F, class Acc>
    [[nodiscard]] Acc fold_edges_e(F&& f, Acc init) const {
        for (const auto& e : storage_.edges()) {
            init = std::invoke(f, e, std::move(init));
        }
        return init;
    }

    template<class F>
    void iter_succ(const Vertex& v, F&& f) const {
        for (const auto& e : storage_.out_edges(v)) std::invoke(f, e.dst);
    }

    template<class F>
    void iter_pred(const Vertex& v, F&& f) const {
        for (const auto& e : storage_.in_edges(v)) std::invoke(f, e.src);
    }

    template<class F>
    void iter_succ_e(const Vertex& v, F&& f) const {
        for (const auto& e : storage_.out_edges(v)) std::invoke(f, e);
    }

    template<class F>
    void iter_pred_e(const Vertex& v, F&& f) const {
        for (const auto& e : storage_.in_edges(v)) std::invoke(f, e);
    }

    [[nodiscard]] std::vector<edge_type> succ_e(const Vertex& v) const { return storage_.out_edges(v); }
    [[nodiscard]] std::vector<edge_type> pred_e(const Vertex& v) const { return storage_.in_edges(v); }

    [[nodiscard]] std::vector<Vertex> succ(const Vertex& v) const {
        std::vector<Vertex> r;
        for (const auto& e : storage_.out_edges(v)) r.push_back(e.dst);
        return r;
    }

    [[nodiscard]] std::vector<Vertex> pred(const Vertex& v) const {
        std::vector<Vertex> r;
        for (const auto& e : storage_.in_edges(v)) r.push_back(e.src);
        return r;
    }

    [[nodiscard]] std::vector<edge_type> find_all_edges(const Vertex& s, const Vertex& d) const {
        return storage_.all_edges(s, d);
    }

    [[nodiscard]] persistent_graph<Vertex, EdgeLabel> freeze() const {
        auto copy = std::make_shared<typename persistent_graph<Vertex, EdgeLabel>::storage_type>(storage_);
        return persistent_graph<Vertex, EdgeLabel>(copy);
    }
};

// ============================================================
// Builders
// Mirrors the spirit of Graph.Builder common construction API
// ============================================================

namespace builder {

template<class Graph>
class imperative_builder {
public:
    using graph_type = Graph;
    using vertex_type = typename graph_type::vertex_type;
    using edge_type = typename graph_type::edge_type;
    using edge_label_type = typename graph_type::edge_label_type;

private:
    graph_type g_;

public:
    imperative_builder() = default;
    explicit imperative_builder(direction dir) : g_(dir) {}

    imperative_builder& vertex(const vertex_type& v) {
        g_.add_vertex(v);
        return *this;
    }

    imperative_builder& edge(const vertex_type& s, const vertex_type& d, const edge_label_type& l = {}) {
        g_.add_edge(s, d, l);
        return *this;
    }

    imperative_builder& edge_e(const edge_type& e) {
        g_.add_edge_e(e);
        return *this;
    }

    [[nodiscard]] graph_type build() const {
        return g_;
    }
};

template<class Graph>
class persistent_builder {
public:
    using graph_type = Graph;
    using vertex_type = typename graph_type::vertex_type;
    using edge_type = typename graph_type::edge_type;
    using edge_label_type = typename graph_type::edge_label_type;

private:
    graph_type g_;

public:
    persistent_builder() = default;
    explicit persistent_builder(direction dir) : g_(dir) {}

    persistent_builder& vertex(const vertex_type& v) {
        g_ = g_.add_vertex(v);
        return *this;
    }

    persistent_builder& edge(const vertex_type& s, const vertex_type& d, const edge_label_type& l = {}) {
        g_ = g_.add_edge(s, d, l);
        return *this;
    }

    persistent_builder& edge_e(const edge_type& e) {
        g_ = g_.add_edge_e(e);
        return *this;
    }

    [[nodiscard]] graph_type build() const {
        return g_;
    }
};

} // namespace builder

// ============================================================
// Algorithms
// ============================================================

namespace algo {

template<class Graph, class Pred>
[[nodiscard]] std::optional<typename Graph::vertex_type>
find_vertex_if(const Graph& g, Pred&& pred) {
    std::optional<typename Graph::vertex_type> out;
    g.iter_vertex([&](const auto& v) {
        if (!out && std::invoke(pred, v)) out = v;
    });
    return out;
}

template<class Graph, class LabelOf>
[[nodiscard]] std::optional<typename Graph::vertex_type>
find_vertex_by_label(const Graph& g, const std::string& wanted, LabelOf&& label_of) {
    return find_vertex_if(g, [&](const auto& v) {
        return std::invoke(label_of, v) == wanted;
    });
}

template<class Graph>
[[nodiscard]] bool reachable(
    const Graph& g,
    const typename Graph::vertex_type& src,
    const typename Graph::vertex_type& dst
) {
    using V = typename Graph::vertex_type;
    if (!g.mem_vertex(src) || !g.mem_vertex(dst)) return false;
    if (src == dst) return true;

    std::queue<V> q;
    std::unordered_set<V> seen;
    q.push(src);
    seen.insert(src);

    while (!q.empty()) {
        auto v = q.front();
        q.pop();
        for (const auto& n : g.succ(v)) {
            if (seen.contains(n)) continue;
            if (n == dst) return true;
            seen.insert(n);
            q.push(n);
        }
    }
    return false;
}

template<class Graph>
[[nodiscard]] std::optional<std::vector<typename Graph::vertex_type>>
shortest_path(
    const Graph& g,
    const typename Graph::vertex_type& src,
    const typename Graph::vertex_type& dst
) {
    using V = typename Graph::vertex_type;
    if (!g.mem_vertex(src) || !g.mem_vertex(dst)) return std::nullopt;

    std::queue<V> q;
    std::unordered_map<V, V> parent;
    std::unordered_set<V> seen;

    q.push(src);
    seen.insert(src);

    bool found = false;
    while (!q.empty() && !found) {
        V cur = q.front();
        q.pop();
        for (const auto& n : g.succ(cur)) {
            if (seen.contains(n)) continue;
            seen.insert(n);
            parent[n] = cur;
            if (n == dst) {
                found = true;
                break;
            }
            q.push(n);
        }
    }

    if (!found && src != dst) return std::nullopt;

    std::vector<V> path;
    V cur = dst;
    path.push_back(cur);
    while (!(cur == src)) {
        cur = parent.at(cur);
        path.push_back(cur);
    }
    std::reverse(path.begin(), path.end());
    return path;
}

template<class Graph>
[[nodiscard]] std::vector<typename Graph::vertex_type>
bfs_order(const Graph& g, const typename Graph::vertex_type& root) {
    using V = typename Graph::vertex_type;
    std::vector<V> out;
    if (!g.mem_vertex(root)) return out;

    std::queue<V> q;
    std::unordered_set<V> seen;
    q.push(root);
    seen.insert(root);

    while (!q.empty()) {
        auto v = q.front();
        q.pop();
        out.push_back(v);
        for (const auto& n : g.succ(v)) {
            if (seen.insert(n).second) q.push(n);
        }
    }
    return out;
}

template<class Graph>
[[nodiscard]] std::size_t degree(const Graph& g, const typename Graph::vertex_type& v) {
    return g.succ(v).size() + g.pred(v).size();
}

template<class Graph>
[[nodiscard]] std::vector<typename Graph::vertex_type>
vertices(const Graph& g) {
    std::vector<typename Graph::vertex_type> out;
    g.iter_vertex([&](const auto& v) { out.push_back(v); });
    return out;
}

template<class Graph>
[[nodiscard]] std::vector<typename Graph::edge_type>
edges(const Graph& g) {
    std::vector<typename Graph::edge_type> out;
    g.iter_edges_e([&](const auto& e) { out.push_back(e); });
    return out;
}

} // namespace algo

// ============================================================
// Queryfitt: AST and execution model
// ============================================================

namespace queryfitt {

enum class query_kind {
    pattern,
    traversal,
    path,
    reachability,
    aggregation
};

struct meta_block {
    std::optional<std::string> name;
    std::optional<std::string> desc;
    std::optional<std::string> graph;
};

struct predicate_expr {
    std::string text;
};

struct pattern_clause {
    std::string vertex_alias = "NODE";
    std::string edge_alias = "EDGE";
    std::size_t count = 0;
    std::string vertex_type;
    std::optional<predicate_expr> edge_predicate;
    std::optional<predicate_expr> where_predicate;
};

struct traversal_clause {
    std::string from;
    std::size_t depth = 1;
    std::optional<predicate_expr> edge_filter;
    std::optional<predicate_expr> vertex_filter;
};

struct path_clause {
    std::string from;
    std::string to;
    bool shortest = true;
    std::optional<predicate_expr> edge_filter;
};

struct reachability_clause {
    std::string from;
    std::string to;
    std::size_t max_depth = 0;
};

struct aggregation_clause {
    std::string op;     // count, sum, avg, min, max, degree, etc
    std::string target; // vertices, edges, paths, matches
    std::optional<predicate_expr> where;
    std::optional<std::string> by;
};

using body_clause = std::variant<
    pattern_clause,
    traversal_clause,
    path_clause,
    reachability_clause,
    aggregation_clause
>;

struct query {
    meta_block meta;
    std::string source;
    query_kind kind {};
    body_clause body;
};

template<class Vertex, class EdgeLabel>
struct match_result {
    using edge_type = edge<Vertex, EdgeLabel>;
    std::vector<Vertex> vertices;
    std::vector<edge_type> edges;
    std::map<std::string, std::string> metadata;
};

template<class Vertex>
struct path_result {
    std::vector<Vertex> path;
};

struct scalar_result {
    std::variant<std::int64_t, double, std::string, bool> value;
};

template<class Vertex, class EdgeLabel>
using result = std::variant<
    std::vector<match_result<Vertex, EdgeLabel>>,
    std::vector<Vertex>,
    std::vector<path_result<Vertex>>,
    bool,
    scalar_result
>;

// ------------------------------------------------------------
// C++-native DSL
// Uses confirmed DSLUtils mixin shape if available;
// otherwise provides a fallback.
// ------------------------------------------------------------

#if GRAFITT_HAS_DSLUTILS

struct QueryfittDSL
    : dsl::DSL<
          QueryfittDSL,
          dsl::Pipeline,
          dsl::Operators,
          dsl::PatternMatch,
          dsl::AST,
          dsl::Rewrite,
          dsl::ExprTemplates,
          dsl::CustomLiterals> {
    using self_type = QueryfittDSL;
};

#endif

struct vertex_ref {
    std::string name;
};

struct edge_ref {
    std::string name;
};

struct native_pattern_builder {
    pattern_clause clause;

    native_pattern_builder& alias_vertex(std::string a) {
        clause.vertex_alias = std::move(a);
        return *this;
    }

    native_pattern_builder& alias_edge(std::string a) {
        clause.edge_alias = std::move(a);
        return *this;
    }

    native_pattern_builder& select_n(std::size_t n, std::string vertex_type) {
        clause.count = n;
        clause.vertex_type = std::move(vertex_type);
        return *this;
    }

    native_pattern_builder& edge_if(std::string pred) {
        clause.edge_predicate = predicate_expr{std::move(pred)};
        return *this;
    }

    native_pattern_builder& where(std::string pred) {
        clause.where_predicate = predicate_expr{std::move(pred)};
        return *this;
    }

    [[nodiscard]] query into_query(std::string source = {}) const {
        query q;
        q.source = std::move(source);
        q.kind = query_kind::pattern;
        q.body = clause;
        return q;
    }
};

inline native_pattern_builder find_pattern() {
    return {};
}

inline query traversal_from(std::string from, std::size_t depth = 1) {
    query q;
    q.kind = query_kind::traversal;
    q.body = traversal_clause{std::move(from), depth, std::nullopt, std::nullopt};
    return q;
}

inline query shortest_path_between(std::string from, std::string to) {
    query q;
    q.kind = query_kind::path;
    q.body = path_clause{std::move(from), std::move(to), true, std::nullopt};
    return q;
}

inline query reachability_between(std::string from, std::string to, std::size_t max_depth = 0) {
    query q;
    q.kind = query_kind::reachability;
    q.body = reachability_clause{std::move(from), std::move(to), max_depth};
    return q;
}

inline query aggregate(std::string op, std::string target) {
    query q;
    q.kind = query_kind::aggregation;
    q.body = aggregation_clause{std::move(op), std::move(target), std::nullopt, std::nullopt};
    return q;
}

// Text parser boundary: Matcheroni-based structure matching can populate this AST.
struct parse_output {
    query value;
    std::size_t consumed = 0;
};

[[nodiscard]] inline std::optional<parse_output> parse_text(std::string_view text) {
    if (text.empty()) return std::nullopt;
#if GRAFITT_HAS_MATCHERONI
    using namespace matcheroni;
    using ws = Any<Atoms<' ', '\t'>>;
    using alpha = Ranges<'a', 'z', 'A', 'Z', '_', '_'>;
    using digit = Range<'0', '9'>;
    using ident = Seq<alpha, Any<alpha, digit>>;
    using path_stmt = Seq<Lit<"path">, ws, ident, ws, Lit<"->">, ws, ident, End>;

    TextMatchContext ctx;
    const auto span = utils::to_span(text);
    if (!path_stmt::match(ctx, span).is_valid()) return std::nullopt;

    auto trimmed = std::string{text};
    auto arrow = trimmed.find("->");
    if (arrow == std::string::npos) return std::nullopt;

    auto lhs = trimmed.substr(4, arrow - 4);
    auto rhs = trimmed.substr(arrow + 2);
    auto trim = [](std::string s) {
        auto is_ws = [](unsigned char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; };
        while (!s.empty() && is_ws(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
        while (!s.empty() && is_ws(static_cast<unsigned char>(s.back()))) s.pop_back();
        return s;
    };
    lhs = trim(std::move(lhs));
    rhs = trim(std::move(rhs));
    if (lhs.empty() || rhs.empty()) return std::nullopt;

    query q;
    q.source = std::string{text};
    q.kind = query_kind::path;
    q.body = path_clause{std::move(lhs), std::move(rhs), true, std::nullopt};
    return parse_output{std::move(q), text.size()};
#else
    (void)text;
    return std::nullopt;
#endif
}

} // namespace queryfitt

// ============================================================
// Rewriting placeholders
// ============================================================

namespace rewrite {

struct rule {
    std::string name;
    queryfitt::query match;
    std::string replacement;
};

template<class Graph>
[[nodiscard]] inline Graph apply_once(const Graph& g, const rule&) {
    return g;
}

} // namespace rewrite

// ============================================================
// Serialization hooks and schema lookup
// ============================================================

namespace gbin {

inline std::filesystem::path schema_root() {
    if (const char* env = std::getenv("GRAFITT_SCHEMA_DIR")) {
        if (*env != '\0') return std::filesystem::path{env};
    }
    return std::filesystem::path{"specs"};
}

inline std::filesystem::path default_schema_path() {
    return schema_root() / "GBIN.sktl";
}

template<class Graph>
[[nodiscard]] inline std::vector<std::uint8_t> serialize(const Graph&) {
    return {};
}

template<class Graph>
[[nodiscard]] inline std::optional<Graph> deserialize(const std::vector<std::uint8_t>&) {
    return std::nullopt;
}

} // namespace gbin

} // namespace grafitt
