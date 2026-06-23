from __future__ import annotations


class DeskDemoError(Exception):
    """Base error for desk workflow."""


class ValidationError(DeskDemoError):
    """Raised when workflow inputs or intermediate data are invalid."""


class RobotMissionError(DeskDemoError):
    """Raised when a robot mission fails."""


class PerceptionError(DeskDemoError):
    """Raised when a perception step fails."""
