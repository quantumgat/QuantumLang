package main

import (
	"encoding/json"
	"log"
	"net/http"
	"os"
	"time"
)

type user struct {
	ID   int64  `json:"id"`
	Name string `json:"name"`
}

type usersResponse struct {
	Users []user `json:"users"`
}

func usersHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodGet {
		http.Error(w, "method not allowed", http.StatusMethodNotAllowed)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	_ = json.NewEncoder(w).Encode(usersResponse{
		Users: []user{{ID: 1, Name: "salam"}},
	})
}

func healthHandler(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "text/plain")
	_, _ = w.Write([]byte("ok"))
}

func main() {
	port := os.Getenv("PORT")
	if port == "" {
		port = "18080"
	}

	mux := http.NewServeMux()
	mux.HandleFunc("/users", usersHandler)
	mux.HandleFunc("/health", healthHandler)

	server := &http.Server{
		Addr:              "127.0.0.1:" + port,
		Handler:           mux,
		ReadHeaderTimeout: 2 * time.Second,
		ReadTimeout:       5 * time.Second,
		WriteTimeout:      5 * time.Second,
		IdleTimeout:       30 * time.Second,
	}

	log.Printf("go_server listening addr=%s", server.Addr)
	if err := server.ListenAndServe(); err != nil && err != http.ErrServerClosed {
		log.Fatal(err)
	}
}
