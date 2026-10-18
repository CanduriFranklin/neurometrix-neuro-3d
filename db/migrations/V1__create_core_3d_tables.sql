-- ============================================================================
-- neurometrix_core_3d - Initial schema
-- ============================================================================
CREATE TABLE IF NOT EXISTS public.jobs (
    id                  BIGSERIAL       PRIMARY KEY,
    message_id          BIGINT          NOT NULL UNIQUE,
    study_uid           TEXT            NOT NULL,
    source_path         TEXT            NOT NULL,
    slice_count         INTEGER         NOT NULL DEFAULT 0,
    status              TEXT            NOT NULL DEFAULT 'queued',
    started_at_utc_ms   BIGINT,
    finished_at_utc_ms  BIGINT,
    processing_ms       BIGINT,
    gpu_device          TEXT,
    error_code          TEXT,
    error_detail        TEXT,
    created_at          TIMESTAMPTZ     NOT NULL DEFAULT NOW(),
    updated_at          TIMESTAMPTZ     NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_core_3d_jobs_status  ON public.jobs (status);
CREATE INDEX IF NOT EXISTS idx_core_3d_jobs_study   ON public.jobs (study_uid);
CREATE INDEX IF NOT EXISTS idx_core_3d_jobs_created ON public.jobs (created_at DESC);

CREATE TABLE IF NOT EXISTS public.results (
    id              BIGSERIAL   PRIMARY KEY,
    job_id          BIGINT      NOT NULL REFERENCES public.jobs(id) ON DELETE CASCADE,
    metric_name     TEXT        NOT NULL,
    metric_value    DOUBLE PRECISION,
    payload         JSONB,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_core_3d_results_job ON public.results (job_id);

-- 3D-specific: volumetric findings
CREATE TABLE IF NOT EXISTS public.findings (
    id              BIGSERIAL   PRIMARY KEY,
    job_id          BIGINT      NOT NULL REFERENCES public.jobs(id) ON DELETE CASCADE,
    finding_type    TEXT        NOT NULL,
    confidence      DOUBLE PRECISION,
    centroid_x      INTEGER,
    centroid_y      INTEGER,
    centroid_z      INTEGER,
    volume_mm3      DOUBLE PRECISION,
    mean_hu         DOUBLE PRECISION,
    note            TEXT,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE INDEX IF NOT EXISTS idx_core_3d_findings_job  ON public.findings (job_id);
CREATE INDEX IF NOT EXISTS idx_core_3d_findings_type ON public.findings (finding_type);