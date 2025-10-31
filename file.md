graph TB
    A[Main Entry Point] --> B[Context Initialization]
    B --> C[System Validation]
    C --> D{Early Return Checks}
    
    D -->|RRD Flag| E[RRD Strategy]
    D -->|Privacy Mode| F[Privacy Strategy]
    D -->|No Previous Logs| G[No Logs Strategy]
    D -->|Continue| H[Strategy Selector]
    
    H --> I[Selected Strategy]
    
    I --> J[Non-DCM Strategy]
    I --> K[OnDemand Strategy]
    I --> L[Reboot Strategy]
    I --> M[DCM Strategy]
    
    J --> N[Upload Execution Engine]
    K --> N
    L --> N
    M --> N
    
    N --> O[Direct Upload Path]
    N --> P[CodeBig Upload Path]
    N --> Q[Fallback Handler]
    
    O --> R[MTLS Authentication]
    P --> S[OAuth Authentication]
    Q --> T[Retry Logic]
    
    R --> U[HTTP/HTTPS Transfer]
    S --> U
    T --> U
    
    U --> V[Upload Verification]
    V --> W[Cleanup & Notification]
    
    subgraph "Support Modules"
        X[Configuration Manager]
        Y[Log Collector]
        Z[File Operations]
        AA[Network Utils]
        BB[Event Manager]
    end
    
    subgraph "Security Layer"
        CC[Certificate Management]
        DD[TLS/MTLS Handler]
        EE[OCSP Validation]
    end
    
    A -.-> X
    B -.-> Y
    I -.-> Z
    N -.-> AA
    W -.-> BB
    
    R -.-> CC
    R -.-> DD
    R -.-> EE
