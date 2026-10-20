#!/usr/bin/env python3
"""
Cross-platform script to synchronize, update, and push git submodules
and update the parent repository submodule pointers.

Works on Windows, macOS, and Linux with Python 3.6+.
"""

import argparse
import os
import subprocess
import sys
from pathlib import Path
from typing import List, Optional, Tuple, Dict


# ANSI colors for friendly terminal output
class Colors:
    RESET = "\033[0m"
    BOLD = "\033[1m"
    GREEN = "\033[32m"
    YELLOW = "\033[33m"
    BLUE = "\033[34m"
    RED = "\033[31m"
    CYAN = "\033[36m"

    @classmethod
    def disable_if_needed(cls):
        # Disable colors if not connected to a tty or if NO_COLOR env var is set
        if not sys.stdout.isatty() or os.environ.get("NO_COLOR"):
            cls.RESET = ""
            cls.BOLD = ""
            cls.GREEN = ""
            cls.YELLOW = ""
            cls.BLUE = ""
            cls.RED = ""
            cls.CYAN = ""


def log_info(msg: str):
    print(f"{Colors.BLUE}[INFO]{Colors.RESET} {msg}")


def log_success(msg: str):
    print(f"{Colors.GREEN}[SUCCESS]{Colors.RESET} {msg}")


def log_warn(msg: str):
    print(f"{Colors.YELLOW}[WARN]{Colors.RESET} {msg}")


def log_error(msg: str):
    print(f"{Colors.RED}[ERROR]{Colors.RESET} {msg}")


def log_step(msg: str):
    print(f"\n{Colors.BOLD}{Colors.CYAN}==> {msg}{Colors.RESET}")


def run_git(args: List[str], cwd: Optional[Path] = None, check: bool = True, capture: bool = True) -> subprocess.CompletedProcess:
    """Run a git command and return CompletedProcess."""
    cmd = ["git"] + args
    res = subprocess.run(
        cmd,
        cwd=cwd,
        capture_output=capture,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False
    )
    if check and res.returncode != 0:
        err_msg = res.stderr.strip() if res.stderr else f"Exit code {res.returncode}"
        raise RuntimeError(f"Command '{' '.join(cmd)}' failed in {cwd or Path.cwd()}:\n{err_msg}")
    return res


def get_repo_root() -> Path:
    """Find root of the current git repository."""
    try:
        res = run_git(["rev-parse", "--show-toplevel"])
        return Path(res.stdout.strip()).resolve()
    except Exception:
        # Fallback to parent of script directory
        return Path(__file__).resolve().parent.parent


def get_submodules(root: Path) -> List[Tuple[str, Path]]:
    """
    Get list of (name, path) tuples for all configured submodules.
    """
    gitmodules_file = root / ".gitmodules"
    if not gitmodules_file.is_file():
        return []

    res = run_git(["config", "--file", ".gitmodules", "--get-regexp", r"submodule\..*\.path"], cwd=root, check=False)
    if res.returncode != 0 or not res.stdout.strip():
        return []

    submodules = []
    for line in res.stdout.strip().splitlines():
        parts = line.strip().split(maxsplit=1)
        if len(parts) == 2:
            key, rel_path = parts
            # key is e.g. submodule.stridejit.path
            name = key.split(".")[1]
            submodules.append((name, (root / rel_path).resolve()))
    return submodules


def is_working_tree_dirty(repo_dir: Path) -> bool:
    """Check if repository has uncommitted tracked changes."""
    # Check unstaged tracked changes
    unstaged = run_git(["diff", "--quiet"], cwd=repo_dir, check=False).returncode != 0
    # Check staged changes
    staged = run_git(["diff", "--cached", "--quiet"], cwd=repo_dir, check=False).returncode != 0
    return unstaged or staged


def get_current_branch(repo_dir: Path) -> Optional[str]:
    """Get currently checked out branch name, or None if in detached HEAD."""
    res = run_git(["branch", "--show-current"], cwd=repo_dir, check=False)
    branch = res.stdout.strip()
    return branch if branch else None


def get_default_remote_branch(repo_dir: Path, remote: str = "origin") -> Optional[str]:
    """Determine default remote branch (e.g. 'main' or 'master')."""
    # Try refs/remotes/<remote>/HEAD
    res = run_git(["symbolic-ref", "--short", f"refs/remotes/{remote}/HEAD"], cwd=repo_dir, check=False)
    if res.returncode == 0 and res.stdout.strip():
        # returns e.g. "origin/main"
        target = res.stdout.strip()
        if target.startswith(f"{remote}/"):
            return target[len(remote) + 1:]
        return target

    # Try common branch names
    for b in ["main", "master", "develop"]:
        r = run_git(["rev-parse", "--verify", f"refs/remotes/{remote}/{b}"], cwd=repo_dir, check=False)
        if r.returncode == 0:
            return b

    return None


def get_upstream_branch(repo_dir: Path) -> Optional[str]:
    """Get the upstream tracking branch for HEAD, e.g. 'origin/main'."""
    res = run_git(["rev-parse", "--abbrev-ref", "--symbolic-full-name", "@{u}"], cwd=repo_dir, check=False)
    if res.returncode == 0 and res.stdout.strip():
        return res.stdout.strip()
    return None


def get_commit_counts(repo_dir: Path, upstream: str) -> Tuple[int, int]:
    """Return (behind_count, ahead_count) relative to upstream."""
    res_behind = run_git(["rev-list", "--count", f"HEAD..{upstream}"], cwd=repo_dir, check=False)
    res_ahead = run_git(["rev-list", "--count", f"{upstream}..HEAD"], cwd=repo_dir, check=False)

    behind = int(res_behind.stdout.strip()) if res_behind.returncode == 0 and res_behind.stdout.strip().isdigit() else 0
    ahead = int(res_ahead.stdout.strip()) if res_ahead.returncode == 0 and res_ahead.stdout.strip().isdigit() else 0
    return behind, ahead


def update_submodule(
    name: str,
    path: Path,
    remote: str,
    push: bool,
    rebase: bool,
    dry_run: bool,
    allow_dirty: bool
) -> Dict[str, any]:
    """
    Synchronize, update, and optionally push a submodule.
    Returns status dict with:
      - 'name': str
      - 'initial_commit': str
      - 'final_commit': str
      - 'updated': bool
      - 'pushed': bool
      - 'warning': Optional[str]
    """
    result = {
        "name": name,
        "initial_commit": "",
        "final_commit": "",
        "updated": False,
        "pushed": False,
        "warning": None,
    }

    if not path.is_dir() or not (path / ".git").exists():
        result["warning"] = f"Submodule path '{path}' is not initialized."
        log_warn(result["warning"])
        return result

    # Initial commit hash
    init_res = run_git(["rev-parse", "HEAD"], cwd=path, check=False)
    initial_commit = init_res.stdout.strip() if init_res.returncode == 0 else ""
    result["initial_commit"] = initial_commit

    # Check for dirty working tree
    dirty = is_working_tree_dirty(path)
    if dirty:
        msg = f"Submodule '{name}' has uncommitted changes to tracked files."
        if not allow_dirty:
            result["warning"] = f"{msg} Skipping sync to avoid data loss."
            log_warn(result["warning"])
            result["final_commit"] = initial_commit
            return result
        else:
            log_warn(f"{msg} Proceeding because --allow-dirty was specified.")

    # Fetch remote
    log_info(f"Fetching {remote} in submodule '{name}'...")
    if not dry_run:
        fetch_res = run_git(["fetch", remote], cwd=path, check=False)
        if fetch_res.returncode != 0:
            result["warning"] = f"Failed to fetch {remote} in '{name}': {fetch_res.stderr.strip()}"
            log_warn(result["warning"])
            result["final_commit"] = initial_commit
            return result

    # Determine branch and upstream
    current_branch = get_current_branch(path)
    if not current_branch:
        default_branch = get_default_remote_branch(path, remote)
        log_warn(f"Submodule '{name}' is in detached HEAD.")
        if default_branch:
            log_info(f"Checking out default branch '{default_branch}' in '{name}'...")
            if not dry_run:
                run_git(["checkout", default_branch], cwd=path, check=False)
            current_branch = default_branch
        else:
            result["warning"] = f"Could not determine branch for '{name}'. Skipping."
            log_warn(result["warning"])
            result["final_commit"] = initial_commit
            return result

    upstream = get_upstream_branch(path)
    if not upstream:
        upstream = f"{remote}/{current_branch}"

    behind, ahead = get_commit_counts(path, upstream)
    log_info(f"Submodule '{name}' on '{current_branch}' vs '{upstream}': {behind} behind, {ahead} ahead.")

    # Sync with upstream
    if behind > 0:
        if ahead == 0:
            log_info(f"Fast-forwarding '{name}' ({behind} commit(s) behind)...")
            if not dry_run:
                ff_res = run_git(["merge", "--ff-only", upstream], cwd=path, check=False)
                if ff_res.returncode != 0:
                    result["warning"] = f"Fast-forward merge failed in '{name}': {ff_res.stderr.strip()}"
                    log_error(result["warning"])
        else:
            if rebase:
                log_info(f"Rebasing '{name}' ({ahead} local commits onto {upstream})...")
                if not dry_run:
                    reb_res = run_git(["rebase", upstream], cwd=path, check=False)
                    if reb_res.returncode != 0:
                        run_git(["rebase", "--abort"], cwd=path, check=False)
                        result["warning"] = f"Rebase conflict in '{name}'. Aborted rebase."
                        log_error(result["warning"])
            else:
                log_info(f"Merging '{upstream}' into '{name}'...")
                if not dry_run:
                    m_res = run_git(["merge", upstream], cwd=path, check=False)
                    if m_res.returncode != 0:
                        run_git(["merge", "--abort"], cwd=path, check=False)
                        result["warning"] = f"Merge conflict in '{name}'. Aborted merge."
                        log_error(result["warning"])

    # Push if ahead
    # Recalculate ahead/behind after possible merge/rebase
    behind_now, ahead_now = get_commit_counts(path, upstream)
    if ahead_now > 0:
        if push:
            log_info(f"Pushing '{name}' to {remote} {current_branch} ({ahead_now} commit(s) ahead)...")
            if not dry_run:
                push_res = run_git(["push", remote, current_branch], cwd=path, check=False)
                if push_res.returncode == 0:
                    result["pushed"] = True
                    log_success(f"Pushed '{name}' successfully.")
                else:
                    result["warning"] = f"Failed to push '{name}': {push_res.stderr.strip()}"
                    log_error(result["warning"])
        else:
            log_info(f"Submodule '{name}' is {ahead_now} commit(s) ahead of {remote} (push disabled).")

    # Final commit hash
    final_res = run_git(["rev-parse", "HEAD"], cwd=path, check=False)
    final_commit = final_res.stdout.strip() if final_res.returncode == 0 else initial_commit
    result["final_commit"] = final_commit
    result["updated"] = (initial_commit != final_commit)

    return result


def main():
    Colors.disable_if_needed()

    parser = argparse.ArgumentParser(
        description="Synchronize, update, and push git submodules and update the parent repository."
    )
    parser.add_argument(
        "--submodule", "-s",
        nargs="+",
        help="Specific submodule name(s) to update. Defaults to all submodules."
    )
    parser.add_argument(
        "--no-push",
        action="store_true",
        help="Do not push submodule or parent repository commits."
    )
    parser.add_argument(
        "--dry-run", "-n",
        action="store_true",
        help="Simulate operations without making changes."
    )
    parser.add_argument(
        "--no-rebase",
        action="store_true",
        help="Use 'git merge' instead of 'git rebase' when local commits have diverged."
    )
    parser.add_argument(
        "--remote",
        default="origin",
        help="Git remote name to fetch from and push to (default: 'origin')."
    )
    parser.add_argument(
        "--commit-msg", "-m",
        help="Custom commit message for parent repository submodule pointer updates."
    )
    parser.add_argument(
        "--no-commit",
        action="store_true",
        help="Do not create a commit in the parent repository."
    )
    parser.add_argument(
        "--allow-dirty",
        action="store_true",
        help="Attempt update even if a submodule has uncommitted changes to tracked files."
    )

    args = parser.parse_args()

    root = get_repo_root()
    log_info(f"Repository root: {root}")

    # Discover submodules
    all_submodules = get_submodules(root)
    if not all_submodules:
        log_warn("No submodules found in .gitmodules.")
        return 0

    submodules_to_process = all_submodules
    if args.submodule:
        target_names = set(args.submodule)
        submodules_to_process = [(n, p) for (n, p) in all_submodules if n in target_names]
        if not submodules_to_process:
            log_error(f"None of the specified submodules {args.submodule} were found.")
            return 1

    push_enabled = not args.no_push
    rebase_enabled = not args.no_rebase

    log_step(f"Processing {len(submodules_to_process)} submodule(s)...")
    results = []
    for name, path in submodules_to_process:
        print(f"\n--- Submodule: {name} ({path.relative_to(root)}) ---")
        res = update_submodule(
            name=name,
            path=path,
            remote=args.remote,
            push=push_enabled,
            rebase=rebase_enabled,
            dry_run=args.dry_run,
            allow_dirty=args.allow_dirty
        )
        results.append(res)

    # Check which submodules have modified commit pointers in parent repo
    log_step("Checking parent repository status...")
    diff_res = run_git(["diff", "--name-only", "--diff-filter=M"], cwd=root, check=False)
    modified_paths = set(diff_res.stdout.strip().splitlines()) if diff_res.stdout.strip() else set()

    submodules_modified_in_parent = []
    for name, path in all_submodules:
        rel_path = str(path.relative_to(root)).replace("\\", "/")
        if rel_path in modified_paths:
            submodules_modified_in_parent.append((name, rel_path))

    if not submodules_modified_in_parent:
        log_success("All submodule pointers in the parent repository are already up to date.")
    else:
        updated_names = [n for (n, _) in submodules_modified_in_parent]
        rel_paths = [p for (_, p) in submodules_modified_in_parent]
        log_info(f"Submodule pointers changed: {', '.join(updated_names)}")

        if args.no_commit:
            log_info("Skipping parent repository commit (--no-commit specified).")
        else:
            default_msg = f"chore(submodules): update {', '.join(updated_names)}"
            commit_msg = args.commit_msg or default_msg
            log_info(f"Committing submodule updates in parent repository: \"{commit_msg}\"")
            if not args.dry_run:
                run_git(["add"] + rel_paths, cwd=root)
                run_git(["commit", "-m", commit_msg], cwd=root)
                log_success("Submodule pointers committed successfully.")

    # Push parent repository if enabled and ahead
    parent_branch = get_current_branch(root)
    if parent_branch:
        parent_upstream = get_upstream_branch(root) or f"{args.remote}/{parent_branch}"
        behind, ahead = get_commit_counts(root, parent_upstream)
        if ahead > 0:
            if push_enabled:
                log_step(f"Pushing parent repository branch '{parent_branch}' to {args.remote} ({ahead} commit(s) ahead)...")
                if not args.dry_run:
                    push_parent = run_git(["push", args.remote, parent_branch], cwd=root, check=False)
                    if push_parent.returncode == 0:
                        log_success(f"Pushed parent repository '{parent_branch}' successfully.")
                    else:
                        log_error(f"Failed to push parent repository:\n{push_parent.stderr.strip()}")
            else:
                log_info(f"Parent repository branch '{parent_branch}' is {ahead} commit(s) ahead of {parent_upstream} (push disabled).")
    else:
        log_warn("Parent repository is in detached HEAD; skipping parent push.")

    # Summary
    log_step("Summary")
    for r in results:
        status_parts = []
        if r["updated"]:
            status_parts.append(f"{Colors.GREEN}updated ({r['initial_commit'][:7]} -> {r['final_commit'][:7]}){Colors.RESET}")
        else:
            status_parts.append("unchanged")
        if r["pushed"]:
            status_parts.append(f"{Colors.GREEN}pushed{Colors.RESET}")
        if r["warning"]:
            status_parts.append(f"{Colors.YELLOW}{r['warning']}{Colors.RESET}")
        print(f"  * {Colors.BOLD}{r['name']}{Colors.RESET}: {', '.join(status_parts)}")

    print(f"\n{Colors.GREEN}Done!{Colors.RESET}\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
