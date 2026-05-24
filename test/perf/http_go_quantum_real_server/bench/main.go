package main

import (
	"flag"
	"fmt"
	"io"
	"net/http"
	"sort"
	"strings"
	"sync"
	"sync/atomic"
	"time"
)

type result struct {
	micros int64
	ok     bool
}

func percentile(sorted []int64, p float64) int64 {
	if len(sorted) == 0 {
		return 0
	}
	index := int(float64(len(sorted)-1) * p)
	return sorted[index]
}

func main() {
	name := flag.String("name", "server", "server name")
	url := flag.String("url", "http://127.0.0.1:18080/users", "benchmark URL")
	requests := flag.Int("requests", 256, "total requests")
	concurrency := flag.Int("concurrency", 8, "client concurrency")
	timeoutMS := flag.Int("timeout-ms", 2000, "per request timeout in milliseconds")
	expect := flag.String("expect", `"users"`, "required response substring")
	flag.Parse()

	if *requests <= 0 || *concurrency <= 0 || *concurrency > *requests {
		fmt.Printf("name=%s ok=false error=invalid-caps requests=%d concurrency=%d\n", *name, *requests, *concurrency)
		return
	}

	client := &http.Client{
		Timeout: time.Duration(*timeoutMS) * time.Millisecond,
		Transport: &http.Transport{
			MaxIdleConns:        *concurrency * 2,
			MaxIdleConnsPerHost: *concurrency * 2,
			MaxConnsPerHost:     *concurrency * 2,
			IdleConnTimeout:     30 * time.Second,
		},
	}

	results := make([]result, *requests)
	var next atomic.Int64
	var errors atomic.Int64
	var wg sync.WaitGroup

	startAll := time.Now()
	for worker := 0; worker < *concurrency; worker++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for {
				index := int(next.Add(1)) - 1
				if index >= *requests {
					return
				}

				start := time.Now()
				resp, err := client.Get(*url)
				elapsed := time.Since(start).Microseconds()
				if err != nil {
					errors.Add(1)
					results[index] = result{micros: elapsed, ok: false}
					continue
				}
				body, readErr := io.ReadAll(resp.Body)
				_ = resp.Body.Close()
				ok := readErr == nil && resp.StatusCode == http.StatusOK && strings.Contains(string(body), *expect)
				if !ok {
					errors.Add(1)
				}
				results[index] = result{micros: elapsed, ok: ok}
			}
		}()
	}
	wg.Wait()
	elapsedAll := time.Since(startAll)

	latencies := make([]int64, 0, *requests)
	for _, row := range results {
		if row.ok {
			latencies = append(latencies, row.micros)
		}
	}
	sort.Slice(latencies, func(i, j int) bool { return latencies[i] < latencies[j] })

	seconds := elapsedAll.Seconds()
	rps := 0.0
	if seconds > 0 {
		rps = float64(len(latencies)) / seconds
	}

	ok := errors.Load() == 0 && len(latencies) == *requests
	fmt.Printf(
		"name=%s ok=%t requests=%d completed=%d concurrency=%d p50Micros=%d p95Micros=%d p99Micros=%d requestsPerSecond=%.2f errors=%d\n",
		*name,
		ok,
		*requests,
		len(latencies),
		*concurrency,
		percentile(latencies, 0.50),
		percentile(latencies, 0.95),
		percentile(latencies, 0.99),
		rps,
		errors.Load(),
	)
}
